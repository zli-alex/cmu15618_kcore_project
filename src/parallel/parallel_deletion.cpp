#include "parallel/parallel_deletion.h"

#include <algorithm>
#include <cmath>
#include <thread>
#include <vector>

#include "sequential/invariants.h"

// Deletion mover construction / round semantics (Week 2 hardening):
//
// The sequential oracle deletion phase is a single-threaded batch: `desire[]`, `moved[]`, and
// `LDSState` levels evolve in deterministic rounds. Parallelism is limited to *read-only* scans
// that assemble the `movers` worklist.
//
// Round state for mover collection (at a fixed target level l):
// - Entry: All demotions for the previous inner-round have finished; all `recompute_desire` calls
//   triggered by that round have finished. `desire`, `moved`, and `state` are mutually consistent.
// - The parallel `collect_movers_at_level_*` phase only *reads* `desire`, `moved`, and `state.level`
//   and appends vertex ids into thread-local buffers. It does not call `set_level` or write
//   `desire`/`moved`.
// - After workers join, results are merged and sorted; the subsequent demotion loop and neighbor
//   recomputes are strictly serial, matching `src/sequential/deletion_phase.cpp`.
//
// Therefore no scan phase performs parallel mutation of shared PLDS structures.

namespace seq {
namespace parallel {
namespace {

int clamp_threads(int num_threads) {
  if (num_threads < 1) {
    return 1;
  }
  return num_threads;
}

int compute_desire_level_vertex(const Graph& graph, const LDSConfig& config,
                                const LDSState& state, VertexId v, int min_allowed_level) {
  const int current = state.level(v);
  if (current <= min_allowed_level) {
    return current;
  }

  for (int target = current - 1; target >= min_allowed_level; --target) {
    if (target <= 0) {
      return 0;
    }

    int deg_z_target_minus_1 = 0;
    for (VertexId u : graph.neighbors(v)) {
      if (state.level(u) >= target - 1) {
        ++deg_z_target_minus_1;
      }
    }

    const int group_i = config.level_to_group(target - 1);
    const double threshold = std::pow(1.0 + std::max(0.0, config.delta), group_i);
    if (static_cast<double>(deg_z_target_minus_1) >= threshold) {
      return target;
    }
  }

  return min_allowed_level;
}

// Read-only w.r.t. LDSState mutation: builds movers from the current round’s desire/moved/levels.
void collect_movers_at_level_serial(const LDSState& state, const std::vector<int>& desire,
                                    const std::vector<bool>& moved, int l,
                                    std::vector<VertexId>* movers) {
  movers->clear();
  for (VertexId v = 0; v < state.num_vertices(); ++v) {
    if (!moved[v] && desire[v] == l && state.level(v) > l) {
      movers->push_back(v);
    }
  }
}

// Parallel read-only classification; shared state must not be mutated until this returns.
void collect_movers_at_level_parallel(const LDSState& state, const std::vector<int>& desire,
                                      const std::vector<bool>& moved, int l, int num_threads,
                                      std::vector<VertexId>* movers) {
  const std::size_t n = state.num_vertices();
  movers->clear();
  if (n == 0) {
    return;
  }

  if (num_threads <= 1) {
    collect_movers_at_level_serial(state, desire, moved, l, movers);
    return;
  }

  std::vector<std::vector<VertexId>> thread_bufs(static_cast<std::size_t>(num_threads));
  std::vector<std::thread> threads;
  threads.reserve(static_cast<std::size_t>(num_threads));

  auto worker = [&](int tid, std::size_t start, std::size_t end) {
    auto& buf = thread_bufs[static_cast<std::size_t>(tid)];
    for (std::size_t idx = start; idx < end; ++idx) {
      const VertexId v = static_cast<VertexId>(idx);
      if (!moved[v] && desire[v] == l && state.level(v) > l) {
        buf.push_back(v);
      }
    }
  };

  const std::size_t nt = static_cast<std::size_t>(num_threads);
  const std::size_t chunk = (n + nt - 1) / nt;
  for (int t = 0; t < num_threads; ++t) {
    const std::size_t start = static_cast<std::size_t>(t) * chunk;
    if (start >= n) {
      break;
    }
    const std::size_t end = std::min(n, start + chunk);
    threads.emplace_back(worker, t, start, end);
  }
  for (auto& th : threads) {
    th.join();
  }

  std::size_t total = 0;
  for (const auto& b : thread_bufs) {
    total += b.size();
  }
  movers->reserve(total);
  for (int t = 0; t < num_threads; ++t) {
    for (VertexId v : thread_bufs[static_cast<std::size_t>(t)]) {
      movers->push_back(v);
    }
  }
  std::sort(movers->begin(), movers->end());
}

}  // namespace

void apply_deletions(Graph& graph, LDSState& state, DescriptorState& descriptors,
                     const LDSConfig& config, const std::vector<EdgeUpdate>& deletions,
                     std::size_t* num_demotions, std::size_t* num_invariant_fixups,
                     int num_threads) {
  const int threads = clamp_threads(num_threads);

  std::vector<VertexId> touched;
  for (const auto& upd : deletions) {
    if (graph.remove_edge(upd.u, upd.v)) {
      touched.push_back(upd.u);
      touched.push_back(upd.v);
    }
  }

  std::sort(touched.begin(), touched.end());
  touched.erase(std::unique(touched.begin(), touched.end()), touched.end());

  std::vector<int> desire(state.num_vertices(), -1);
  std::vector<bool> moved(state.num_vertices(), false);

  auto recompute_desire = [&](VertexId v, int min_allowed_level) {
    if (moved[v] || state.level(v) <= min_allowed_level) {
      desire[v] = -1;
      return;
    }
    if (check_invariant2_vertex(graph, config, state, v)) {
      desire[v] = -1;
      return;
    }
    desire[v] = compute_desire_level_vertex(graph, config, state, v, min_allowed_level);
  };

  for (VertexId v : touched) {
    recompute_desire(v, 0);
  }

  std::vector<VertexId> movers;

  for (int l = 0; l < config.num_levels; ++l) {
    if (threads <= 1) {
      collect_movers_at_level_serial(state, desire, moved, l, &movers);
    } else {
      collect_movers_at_level_parallel(state, desire, moved, l, threads, &movers);
    }

    while (!movers.empty()) {
      // Serial demotion round: mutate levels/desire/moved, then serial neighbor recomputes.
      std::vector<VertexId> moved_this_round;
      for (VertexId v : movers) {
        if (moved[v] || state.level(v) <= l || desire[v] != l) {
          continue;
        }
        const int old_level = state.level(v);
        descriptors.on_level_mutation_start(v, old_level);
        state.set_level(v, l);
        moved[v] = true;
        desire[v] = -1;
        moved_this_round.push_back(v);
        *num_demotions += static_cast<std::size_t>(old_level - l);
        *num_invariant_fixups += static_cast<std::size_t>(old_level - l);
      }

      movers.clear();
      for (VertexId v : moved_this_round) {
        for (VertexId u : graph.neighbors(v)) {
          if (state.level(u) > l) {
            recompute_desire(u, l + 1);
          }
        }
      }

      if (threads <= 1) {
        collect_movers_at_level_serial(state, desire, moved, l, &movers);
      } else {
        collect_movers_at_level_parallel(state, desire, moved, l, threads, &movers);
      }
    }
  }
}

}  // namespace parallel
}  // namespace seq
