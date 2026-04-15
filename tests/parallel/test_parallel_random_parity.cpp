#include <cstdint>
#include <random>
#include <utility>
#include <vector>

#include "core/batch_update.h"
#include "parallel/parallel_batch_engine.h"
#include "sequential/sequential_batch_engine.h"

#include "test_common.h"

namespace {

// Fixed seed for reproducible CI / local runs (change only when intentionally updating coverage).
constexpr std::uint32_t kRandomParitySeed = 0xC001D00Du;
constexpr int kNumTrials = 96;

struct Scenario {
  int n = 0;
  std::vector<std::pair<int, int>> initial_edges;
  seq::BatchUpdate batch;
};

struct RunOutcome {
  seq::BatchResult result;
  seq::DescriptorState descriptors;
};

seq::Graph make_graph(int n, const std::vector<std::pair<int, int>>& edges) {
  seq::Graph g(static_cast<std::size_t>(n));
  for (const auto& e : edges) {
    (void)g.add_edge(static_cast<seq::VertexId>(e.first), static_cast<seq::VertexId>(e.second));
  }
  return g;
}

seq::LDSConfig make_default_config() {
  seq::LDSConfig config;
  config.num_levels = 8;
  config.group_size = 2;
  config.delta = 0.2;
  config.lambda = 9.0;
  return config;
}

Scenario make_scenario(std::mt19937& rng) {
  std::uniform_int_distribution<int> n_dist(4, 14);
  const int n = n_dist(rng);

  std::vector<std::pair<int, int>> edges;
  std::uniform_real_distribution<double> u01(0.0, 1.0);
  for (int u = 0; u < n; ++u) {
    for (int v = u + 1; v < n; ++v) {
      if (u01(rng) < 0.28) {
        edges.push_back({u, v});
      }
    }
  }

  seq::BatchUpdate batch;
  std::uniform_int_distribution<int> len_dist(2, 14);
  std::uniform_int_distribution<int> vert(0, n - 1);
  std::uniform_int_distribution<int> coin(0, 1);
  const int batch_len = len_dist(rng);
  batch.updates.reserve(static_cast<std::size_t>(batch_len));
  for (int i = 0; i < batch_len; ++i) {
    int u = 0;
    int v = 0;
    do {
      u = vert(rng);
      v = vert(rng);
    } while (u == v);
    const seq::UpdateType ty =
        coin(rng) == 0 ? seq::UpdateType::Insert : seq::UpdateType::Delete;
    batch.updates.push_back(
        {ty, static_cast<seq::VertexId>(u), static_cast<seq::VertexId>(v)});
  }

  Scenario s;
  s.n = n;
  s.initial_edges = std::move(edges);
  s.batch = std::move(batch);
  return s;
}

RunOutcome run_sequential(const Scenario& scenario, const seq::LDSConfig& config) {
  seq::Graph graph = make_graph(scenario.n, scenario.initial_edges);
  seq::LDSState state(static_cast<std::size_t>(scenario.n), config, 0);
  seq::DescriptorState descriptors(static_cast<std::size_t>(scenario.n));
  seq::SequentialBatchEngine engine(std::move(graph), config, std::move(state), std::move(descriptors));

  seq::BatchUpdate empty;
  (void)seq::process_batch(engine, empty);

  RunOutcome out;
  out.result = seq::process_batch(engine, scenario.batch);
  out.descriptors = engine.descriptors;
  return out;
}

RunOutcome run_parallel(const Scenario& scenario, const seq::LDSConfig& config, int num_threads) {
  seq::Graph graph = make_graph(scenario.n, scenario.initial_edges);
  seq::LDSState state(static_cast<std::size_t>(scenario.n), config, 0);
  seq::DescriptorState descriptors(static_cast<std::size_t>(scenario.n));
  seq::SequentialBatchEngine seq_engine(std::move(graph), config, std::move(state), std::move(descriptors));

  seq::parallel::ParallelOptions popts;
  popts.num_threads = num_threads;
  seq::parallel::ParallelBatchEngine engine(std::move(seq_engine), popts);

  seq::BatchUpdate empty;
  (void)seq::parallel::process_batch(engine, empty);

  RunOutcome out;
  out.result = seq::parallel::process_batch(engine, scenario.batch);
  out.descriptors = engine.sequential.descriptors;
  return out;
}

void assert_batch_close(const seq::BatchResult& expected, const seq::BatchResult& actual) {
  EXPECT_VEC_EQ(expected.levels_before, actual.levels_before);
  EXPECT_VEC_EQ(expected.levels_after, actual.levels_after);
  EXPECT_VEC_NEAR(expected.coreness_estimate, actual.coreness_estimate, 1e-12);
  EXPECT_EQ(expected.num_promotions, actual.num_promotions);
  EXPECT_EQ(expected.num_demotions, actual.num_demotions);
  EXPECT_EQ(expected.invariant_fixups, actual.invariant_fixups);
  EXPECT_EQ(expected.insertion_upward_moves, actual.insertion_upward_moves);
  EXPECT_EQ(expected.insertion_downward_moves, actual.insertion_downward_moves);
  EXPECT_EQ(expected.deletion_upward_moves, actual.deletion_upward_moves);
  EXPECT_EQ(expected.deletion_downward_moves, actual.deletion_downward_moves);
  EXPECT_EQ(expected.invariants_hold_after_insertions, actual.invariants_hold_after_insertions);
  EXPECT_EQ(expected.invariants_hold_after_deletions, actual.invariants_hold_after_deletions);
  EXPECT_VEC_EQ(expected.moved_vertices, actual.moved_vertices);
}

void assert_descriptor_finalized(const RunOutcome& out) {
  for (std::size_t v = 0; v < out.descriptors.marked.size(); ++v) {
    EXPECT_EQ(static_cast<int>(out.descriptors.marked[v]), 0);
  }
  for (seq::VertexId v : out.result.moved_vertices) {
    EXPECT_EQ(static_cast<int>(out.descriptors.first_mutation_recorded[v]), 1);
  }
}

}  // namespace

int main() {
  std::mt19937 rng(kRandomParitySeed);
  const seq::LDSConfig config = make_default_config();

  for (int trial = 0; trial < kNumTrials; ++trial) {
    const Scenario scenario = make_scenario(rng);

    const RunOutcome seq_out = run_sequential(scenario, config);
    const RunOutcome par1_out = run_parallel(scenario, config, 1);
    const RunOutcome par4_out = run_parallel(scenario, config, 4);

    assert_batch_close(seq_out.result, par1_out.result);
    assert_batch_close(par1_out.result, par4_out.result);
    assert_descriptor_finalized(seq_out);
    assert_descriptor_finalized(par1_out);
    assert_descriptor_finalized(par4_out);
  }

  return 0;
}
