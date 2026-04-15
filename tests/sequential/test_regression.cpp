#include <fstream>
#include <sstream>
#include <string>

#include "core/batch_update.h"
#include "sequential/sequential_batch_engine.h"

#include "test_common.h"

static seq::Graph load_graph(const std::string& path, std::size_t n) {
  seq::Graph g(n);
  std::ifstream in(path);
  std::size_t u = 0;
  std::size_t v = 0;
  while (in >> u >> v) {
    g.add_edge(u, v);
  }
  return g;
}

static seq::BatchUpdate load_batch(const std::string& path) {
  seq::BatchUpdate batch;
  std::ifstream in(path);
  char op;
  std::size_t u = 0;
  std::size_t v = 0;
  while (in >> op >> u >> v) {
    batch.updates.push_back({op == 'I' ? seq::UpdateType::Insert : seq::UpdateType::Delete, u, v});
  }
  return batch;
}

int main() {
  const std::string graph_path = "datasets/tiny/triangle.graph";
  const std::string batch_path = "datasets/tiny/mixed.batch";

  seq::LDSConfig cfg;
  cfg.num_levels = 8;
  cfg.group_size = 2;
  cfg.delta = 0.2;
  cfg.lambda = 9.0;

  seq::Graph g = load_graph(graph_path, 4);
  seq::LDSState s(4, cfg, 0);
  seq::DescriptorState d(4);
  seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

  auto batch = load_batch(batch_path);
  auto result1 = seq::process_batch(engine, batch);
  auto snapshot = result1.coreness_estimate;
  auto result2 = seq::process_batch(engine, batch);

  EXPECT_EQ(result2.coreness_estimate.size(), snapshot.size());
  EXPECT_EQ(engine.descriptors.batch_number, static_cast<std::size_t>(2));
  return 0;
}
