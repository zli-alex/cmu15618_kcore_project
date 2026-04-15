#include <algorithm>
#include <fstream>
#include <thread>
#include <vector>

#include "core/batch_update.h"
#include "parallel/parallel_batch_engine.h"
#include "sequential/sequential_batch_engine.h"

#include "test_common.h"

namespace {

std::size_t detect_num_vertices(const std::string& graph_path, const std::string& batch_path) {
  std::size_t max_vertex = 0;

  {
    std::ifstream in(graph_path);
    std::size_t u = 0;
    std::size_t v = 0;
    while (in >> u >> v) {
      max_vertex = std::max(max_vertex, std::max(u, v));
    }
  }

  {
    std::ifstream in(batch_path);
    char op = 'I';
    std::size_t u = 0;
    std::size_t v = 0;
    while (in >> op >> u >> v) {
      max_vertex = std::max(max_vertex, std::max(u, v));
    }
  }

  return max_vertex + 1;
}

seq::Graph load_graph(const std::string& path, std::size_t num_vertices) {
  seq::Graph graph(num_vertices);
  std::ifstream in(path);
  std::size_t u = 0;
  std::size_t v = 0;
  while (in >> u >> v) {
    graph.add_edge(u, v);
  }
  return graph;
}

seq::BatchUpdate load_batch(const std::string& path) {
  seq::BatchUpdate batch;
  std::ifstream in(path);
  char op = 'I';
  std::size_t u = 0;
  std::size_t v = 0;
  while (in >> op >> u >> v) {
    batch.updates.push_back(
        {op == 'I' ? seq::UpdateType::Insert : seq::UpdateType::Delete, u, v});
  }
  return batch;
}

seq::LDSConfig make_default_config() {
  seq::LDSConfig config;
  config.num_levels = 8;
  config.group_size = 2;
  config.delta = 0.2;
  config.lambda = 9.0;
  return config;
}

struct RunOutcome {
  seq::BatchResult result;
  seq::DescriptorState descriptors;
};

RunOutcome run_sequential_on_trace(const std::string& graph_path, const std::string& batch_path,
                                   const seq::LDSConfig& config) {
  const std::size_t n = detect_num_vertices(graph_path, batch_path);
  seq::Graph graph = load_graph(graph_path, n);
  seq::LDSState state(n, config, 0);
  seq::DescriptorState descriptors(n);
  seq::SequentialBatchEngine engine(std::move(graph), config, std::move(state), std::move(descriptors));

  seq::BatchUpdate empty;
  (void)seq::process_batch(engine, empty);

  const seq::BatchUpdate batch = load_batch(batch_path);
  RunOutcome out;
  out.result = seq::process_batch(engine, batch);
  out.descriptors = engine.descriptors;
  return out;
}

RunOutcome run_parallel_on_trace(const std::string& graph_path, const std::string& batch_path,
                                 const seq::LDSConfig& config, int num_threads) {
  const std::size_t n = detect_num_vertices(graph_path, batch_path);
  seq::Graph graph = load_graph(graph_path, n);
  seq::LDSState state(n, config, 0);
  seq::DescriptorState descriptors(n);
  seq::SequentialBatchEngine seq_engine(std::move(graph), config, std::move(state), std::move(descriptors));

  seq::parallel::ParallelOptions popts;
  popts.num_threads = num_threads;
  seq::parallel::ParallelBatchEngine engine(std::move(seq_engine), popts);

  seq::BatchUpdate empty;
  (void)seq::parallel::process_batch(engine, empty);

  const seq::BatchUpdate batch = load_batch(batch_path);
  RunOutcome out;
  out.result = seq::parallel::process_batch(engine, batch);
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
  const seq::LDSConfig config = make_default_config();

  const std::vector<std::string> traces = {
      "datasets/tiny/fixed_traces/trace_triangle_insert",
      "datasets/tiny/fixed_traces/trace_triangle_delete",
      "datasets/tiny/fixed_traces/trace_path_mixed",
      "datasets/tiny/fixed_traces/trace_batch_only_edges",
      "datasets/tiny/fixed_traces/trace_star_mixed",
      "datasets/tiny/fixed_traces/trace_cycle4_mixed",
      "datasets/tiny/fixed_traces/trace_path_chord",
      "datasets/tiny/fixed_traces/trace_ks4_one_del",
  };

  const unsigned int hc = std::thread::hardware_concurrency();
  const int par_n_threads = static_cast<int>(std::max(1u, std::min(4u, hc == 0 ? 4u : hc)));

  for (const auto& base : traces) {
    const std::string graph_path = base + ".graph";
    const std::string batch_path = base + ".batch";

    std::ifstream probe(graph_path);
    EXPECT_TRUE(static_cast<bool>(probe));

    const RunOutcome seq_out = run_sequential_on_trace(graph_path, batch_path, config);
    const RunOutcome par1_out = run_parallel_on_trace(graph_path, batch_path, config, 1);
    const RunOutcome parn_out = run_parallel_on_trace(graph_path, batch_path, config, par_n_threads);

    assert_batch_close(seq_out.result, par1_out.result);
    assert_batch_close(par1_out.result, parn_out.result);
    assert_descriptor_finalized(seq_out);
    assert_descriptor_finalized(par1_out);
    assert_descriptor_finalized(parn_out);
  }

  return 0;
}
