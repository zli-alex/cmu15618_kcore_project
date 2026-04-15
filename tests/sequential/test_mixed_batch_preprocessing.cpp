#include "core/batch_update.h"
#include "sequential/sequential_batch_engine.h"

#include "test_common.h"

int main() {
  seq::LDSConfig cfg;
  cfg.num_levels = 8;
  cfg.group_size = 2;
  cfg.delta = 0.2;
  cfg.lambda = 9.0;

  // Duplicate insertions are deterministic and idempotent at graph level.
  {
    seq::Graph g(2);
    seq::LDSState s(2, cfg, 0);
    seq::DescriptorState d(2);
    seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

    seq::BatchUpdate batch{{{seq::UpdateType::Insert, 0, 1},
                            {seq::UpdateType::Insert, 0, 1}}};
    (void)seq::process_batch(engine, batch);
    EXPECT_TRUE(engine.graph.has_edge(0, 1));
  }

  // Duplicate deletions are deterministic and idempotent at graph level.
  {
    seq::Graph g(2);
    g.add_edge(0, 1);
    seq::LDSState s(2, cfg, 0);
    seq::DescriptorState d(2);
    seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

    seq::BatchUpdate batch{{{seq::UpdateType::Delete, 0, 1},
                            {seq::UpdateType::Delete, 0, 1}}};
    (void)seq::process_batch(engine, batch);
    EXPECT_TRUE(!engine.graph.has_edge(0, 1));
  }

  // Insert+delete of the same edge in one mixed batch: net absent.
  {
    seq::Graph g(2);
    seq::LDSState s(2, cfg, 0);
    seq::DescriptorState d(2);
    seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

    seq::BatchUpdate batch{{{seq::UpdateType::Insert, 0, 1},
                            {seq::UpdateType::Delete, 0, 1}}};
    (void)seq::process_batch(engine, batch);
    EXPECT_TRUE(!engine.graph.has_edge(0, 1));
  }

  // Self-loop insert should be ignored by Graph::add_edge.
  {
    seq::Graph g(2);
    seq::LDSState s(2, cfg, 0);
    seq::DescriptorState d(2);
    seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

    seq::BatchUpdate batch{{{seq::UpdateType::Insert, 0, 0}}};
    (void)seq::process_batch(engine, batch);
    EXPECT_TRUE(!engine.graph.has_edge(0, 0));
  }

  return 0;
}
