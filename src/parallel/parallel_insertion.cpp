#include "parallel/parallel_insertion.h"

#include <algorithm>
#include <thread>
#include <vector>

#include "sequential/invariants.h"

// Snapshot / scan / apply discipline (Week 2 hardening):
//
// - Active-level snapshot: For each level l, we copy `state.vertices_in_level(l)` into a local
//   vector and sort it ascending. That list is the only vertex set the scan considers for level l
//   (matching the sequential oracle’s “candidates from bucket l” discipline).
// - Read-only scan: Parallel and serial violation scans call only `state.level`, neighbor iteration
//   via Invariant 1 checks, and never call `LDSState::set_level` or `DescriptorState` mutation.
//   No graph edge mutations happen during the scan.
// - Serial apply: Promotions run on the main thread after all scan workers join, so shared LDS
//   bucket structure updates never race with scan readers.

namespace seq {
namespace parallel {
namespace {

int clamp_threads(int num_threads) {
  if (num_threads < 1) {
    return 1;
  }
  return num_threads;
}

void scan_violations_serial(const Graph& graph, const LDSConfig& config, const LDSState& state,
                            int l, const std::vector<VertexId>& active_level_snapshot,
                            std::vector<VertexId>* out) {
  out->clear();
  for (VertexId v : active_level_snapshot) {
    if (state.level(v) != l) {
      continue;
    }
    if (!check_invariant1_vertex(graph, config, state, v)) {
      out->push_back(v);
    }
  }
}

void scan_violations_parallel(const Graph& graph, const LDSConfig& config, const LDSState& state,
                              int l, const std::vector<VertexId>& active_level_snapshot,
                              int num_threads, std::vector<VertexId>* out) {
  const std::size_t m = active_level_snapshot.size();
  out->clear();
  if (m == 0) {
    return;
  }

  std::vector<std::vector<VertexId>> thread_bufs(static_cast<std::size_t>(num_threads));
  std::vector<std::thread> threads;
  threads.reserve(static_cast<std::size_t>(num_threads));

  auto worker = [&](int tid, std::size_t start, std::size_t end) {
    auto& buf = thread_bufs[static_cast<std::size_t>(tid)];
    for (std::size_t i = start; i < end; ++i) {
      const VertexId v = active_level_snapshot[i];
      if (state.level(v) != l) {
        continue;
      }
      if (!check_invariant1_vertex(graph, config, state, v)) {
        buf.push_back(v);
      }
    }
  };

  const std::size_t nt = static_cast<std::size_t>(num_threads);
  const std::size_t chunk = (m + nt - 1) / nt;
  for (int t = 0; t < num_threads; ++t) {
    const std::size_t start = static_cast<std::size_t>(t) * chunk;
    if (start >= m) {
      break;
    }
    const std::size_t end = std::min(m, start + chunk);
    threads.emplace_back(worker, t, start, end);
  }
  for (auto& th : threads) {
    th.join();
  }

  std::size_t total = 0;
  for (const auto& b : thread_bufs) {
    total += b.size();
  }
  out->reserve(total);
  for (int t = 0; t < num_threads; ++t) {
    for (VertexId v : thread_bufs[static_cast<std::size_t>(t)]) {
      out->push_back(v);
    }
  }
  std::sort(out->begin(), out->end());
}

}  // namespace

void apply_insertions(Graph& graph, LDSState& state, DescriptorState& descriptors,
                      const LDSConfig& config, const std::vector<EdgeUpdate>& insertions,
                      std::size_t* num_promotions, std::size_t* num_invariant_fixups,
                      int num_threads) {
  for (const auto& upd : insertions) {
    (void)graph.add_edge(upd.u, upd.v);
  }

  const int threads = clamp_threads(num_threads);
  std::vector<VertexId> promote_list;
  std::vector<VertexId> scan_buf;

  for (int l = 0; l < config.num_levels; ++l) {
    // Copy + sort: explicit snapshot of bucket(l) at this level sweep (oracle-aligned).
    std::vector<VertexId> active_level_snapshot(state.vertices_in_level(l));
    std::sort(active_level_snapshot.begin(), active_level_snapshot.end());

    if (threads <= 1) {
      scan_violations_serial(graph, config, state, l, active_level_snapshot, &scan_buf);
      promote_list.swap(scan_buf);
    } else {
      scan_violations_parallel(graph, config, state, l, active_level_snapshot, threads,
                               &promote_list);
    }

    // Serial mutation phase only (after any parallel scan has joined).
    for (VertexId v : promote_list) {
      if (state.level(v) != l) {
        continue;
      }
      if (!check_invariant1_vertex(graph, config, state, v)) {
        const int level_before = state.level(v);
        if (promote_vertex_one_level(graph, config, state, v)) {
          ++(*num_promotions);
          ++(*num_invariant_fixups);
          descriptors.on_level_mutation_start(v, level_before);
        }
      }
    }
  }
}

}  // namespace parallel
}  // namespace seq
