#include <vector>

#include "core/batch_update.h"
#include "sequential/invariants.h"
#include "sequential/sequential_batch_engine.h"

#include "test_common.h"

int main() {
  {
    seq::LDSConfig cfg;
    cfg.num_levels = 8;
    cfg.group_size = 2;
    cfg.lambda = 9.0;
    cfg.delta = 0.2;

    seq::Graph g(3);
    seq::LDSState s(3, cfg, 0);
    seq::DescriptorState d(3);
    seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

    seq::BatchUpdate insert_batch{{{seq::UpdateType::Insert, 0, 1}}};
    const auto result = seq::process_batch(engine, insert_batch);

    EXPECT_EQ(result.insertion_downward_moves, static_cast<std::size_t>(0));
    EXPECT_EQ(result.deletion_upward_moves, static_cast<std::size_t>(0));
    EXPECT_TRUE(result.invariants_hold_after_insertions);
    EXPECT_TRUE(result.invariants_hold_after_deletions);
    EXPECT_TRUE(seq::all_vertices_satisfy_invariants(engine.graph, engine.config,
                                                     engine.state));
  }

  {
    seq::LDSConfig cfg;
    cfg.num_levels = 8;
    cfg.group_size = 2;
    cfg.lambda = 9.0;
    cfg.delta = 0.2;

    seq::Graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    seq::LDSState s(3, cfg, 0);
    seq::DescriptorState d(3);
    seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

    seq::BatchUpdate delete_batch{{{seq::UpdateType::Delete, 0, 1}}};
    const auto result = seq::process_batch(engine, delete_batch);

    EXPECT_EQ(result.deletion_upward_moves, static_cast<std::size_t>(0));
    EXPECT_EQ(result.insertion_downward_moves, static_cast<std::size_t>(0));
    EXPECT_TRUE(result.invariants_hold_after_insertions);
    EXPECT_TRUE(result.invariants_hold_after_deletions);
    EXPECT_TRUE(seq::all_vertices_satisfy_invariants(engine.graph, engine.config,
                                                     engine.state));
  }

  return 0;
}
