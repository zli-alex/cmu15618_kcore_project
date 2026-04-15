#include "core/batch_update.h"
#include "sequential/coreness.h"
#include "sequential/lds_state.h"
#include "sequential/read_interface.h"
#include "sequential/sequential_batch_engine.h"

#include "test_common.h"

int main() {
  {
    seq::LDSConfig cfg;
    cfg.num_levels = 8;
    cfg.group_size = 2;
    cfg.delta = 0.2;
    cfg.lambda = 9.0;

    seq::Graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(0, 2);
    seq::LDSState s(3, cfg, 2);
    seq::DescriptorState d(3);
    seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

    seq::BatchUpdate batch{{{seq::UpdateType::Delete, 0, 2}}};
    (void)seq::process_batch(engine, batch);

    for (seq::VertexId v = 0; v < engine.state.num_vertices(); ++v) {
      seq::ReadDebugInfo dbg;
      const double got = seq::read_estimate(engine, v, &dbg);
      const double expected = seq::estimate_coreness_from_level(
          engine.config, engine.state.level(v), engine.state.num_vertices());
      EXPECT_NEAR(got, expected, 1e-9);
      EXPECT_EQ(dbg.batch_before, dbg.batch_after);
      EXPECT_TRUE(dbg.dag_status == seq::DagMarkStatus::Unmarked);
      EXPECT_EQ(dbg.root, seq::find_root(engine.descriptors, v));
    }
  }

  {
    seq::DescriptorState d(4);
    d.parent[0] = 1;
    d.parent[1] = 2;
    d.parent[2] = 2;
    d.parent[3] = 3;

    EXPECT_EQ(seq::find_root(d, 0), static_cast<seq::VertexId>(2));
    EXPECT_EQ(seq::find_root(d, 1), static_cast<seq::VertexId>(2));
    EXPECT_EQ(seq::find_root(d, 3), static_cast<seq::VertexId>(3));
    EXPECT_TRUE(seq::check_dag(d, 0) == seq::DagMarkStatus::Unmarked);
    d.marked[2] = 1;
    EXPECT_TRUE(seq::check_dag(d, 0) == seq::DagMarkStatus::Marked);
    EXPECT_TRUE(seq::check_dag(d, 1) == seq::DagMarkStatus::Marked);
    d.marked[2] = 0;
    EXPECT_TRUE(seq::check_dag(d, 0) == seq::DagMarkStatus::Unmarked);
    d.marked[0] = 1;
    EXPECT_TRUE(seq::check_dag(d, 0) == seq::DagMarkStatus::Unmarked);
  }

  {
    seq::LDSConfig cfg;
    // Wide enough level range that distinct old_level values are not clamped together
    // (estimate_coreness_from_level clamps to [0, num_levels-1]).
    cfg.num_levels = 64;
    cfg.group_size = 2;
    cfg.delta = 0.2;
    cfg.lambda = 9.0;

    seq::LDSState state(3, cfg, 0);
    state.set_level(0, 5);
    state.set_level(1, 5);
    state.set_level(2, 5);

    seq::DescriptorState d(3);
    d.parent[0] = 2;
    d.parent[1] = 2;
    d.parent[2] = 2;
    // Same marked root (2); per-vertex old_level must drive Marked reads (pick levels
    // in different LDS buckets so coreness estimates differ for this n and cfg).
    constexpr int k_old0 = 0;
    // For n=3, delta=0.2: levels_per_group = int(4*ceil(log(3)/log(1.2))) = 28, so
    // level 0 and 55 land in different buckets after clamping within num_levels.
    constexpr int k_old1 = 55;
    d.old_level[0] = k_old0;
    d.old_level[1] = k_old1;
    d.old_level[2] = 0;
    d.marked[2] = 1;

    const double expect0 =
        seq::estimate_coreness_from_level(cfg, k_old0, state.num_vertices());
    const double expect1 =
        seq::estimate_coreness_from_level(cfg, k_old1, state.num_vertices());
    EXPECT_NE(expect0, expect1);

    const double g0 = seq::read_estimate(cfg, state, d, 0, nullptr);
    const double g1 = seq::read_estimate(cfg, state, d, 1, nullptr);
    EXPECT_NEAR(g0, expect0, 1e-9);
    EXPECT_NEAR(g1, expect1, 1e-9);
    // Non-root vertices 0 and 1 share root 2; read path reports same root as find_root/check_dag.
    seq::ReadDebugInfo dbg0;
    seq::ReadDebugInfo dbg1;
    (void)seq::read_estimate(cfg, state, d, 0, &dbg0);
    (void)seq::read_estimate(cfg, state, d, 1, &dbg1);
    EXPECT_EQ(dbg0.root, static_cast<seq::VertexId>(2));
    EXPECT_EQ(dbg1.root, static_cast<seq::VertexId>(2));
    EXPECT_EQ(dbg0.root, seq::find_root(d, 0));
    EXPECT_EQ(dbg1.root, seq::find_root(d, 1));
    EXPECT_TRUE(dbg0.dag_status == seq::DagMarkStatus::Marked);
    EXPECT_TRUE(dbg1.dag_status == seq::DagMarkStatus::Marked);

    d.marked[2] = 0;
    EXPECT_NEAR(seq::read_estimate(cfg, state, d, 0, nullptr),
                seq::estimate_coreness_from_level(cfg, 5, state.num_vertices()), 1e-9);

    d.marked[2] = 1;
    EXPECT_NEAR(seq::read_estimate(cfg, state, d, 0, nullptr), expect0, 1e-9);
    d.marked[2] = 0;
    EXPECT_NEAR(seq::read_estimate(cfg, state, d, 0, nullptr),
                seq::estimate_coreness_from_level(cfg, 5, state.num_vertices()), 1e-9);

    d.marked[2] = 1;
    seq::ReadDebugInfo dbg;
    (void)seq::read_estimate(cfg, state, d, 0, &dbg);
    EXPECT_EQ(dbg.batch_before, dbg.batch_after);
  }

  return 0;
}
