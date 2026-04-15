#include <vector>

#include "core/batch_update.h"
#include "sequential/coreness.h"
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
    }
  }

  {
    seq::DescriptorState d(4);
    d.parent[0] = 1;
    d.parent[1] = 2;
    d.parent[2] = 2;
    d.parent[3] = 3;

    EXPECT_EQ(seq::find_root(d, 0), static_cast<seq::VertexId>(2));
    EXPECT_EQ(seq::find_root(d, 3), static_cast<seq::VertexId>(3));
    EXPECT_TRUE(seq::check_dag(d, 0) == seq::DagMarkStatus::Unmarked);
  }

  return 0;
}
