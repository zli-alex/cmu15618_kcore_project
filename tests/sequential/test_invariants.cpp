#include "core/graph.h"
#include "sequential/invariants.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

#include "test_common.h"

int main() {
  {
    seq::LDSConfig cfg;
    cfg.num_levels = 8;
    cfg.group_size = 2;
    cfg.delta = 0.2;
    cfg.lambda = 9.0;

    seq::Graph g(4);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(1, 3);

    seq::LDSState state(4, cfg, 1);
    EXPECT_TRUE(!seq::check_invariant1_vertex(g, cfg, state, 1));
    EXPECT_TRUE(seq::promote_vertex_one_level(g, cfg, state, 1));
    EXPECT_EQ(state.level(1), 2);
  }

  {
    seq::LDSConfig cfg;
    cfg.num_levels = 8;
    cfg.group_size = 2;
    cfg.delta = 0.2;
    cfg.lambda = 9.0;

    seq::Graph g(3);
    g.add_edge(0, 1);
    seq::LDSState state(3, cfg, 0);
    state.set_level(2, 1);

    EXPECT_TRUE(!seq::check_invariant2_vertex(g, cfg, state, 2));
    EXPECT_TRUE(seq::demote_vertex_one_level(g, cfg, state, 2));
    EXPECT_EQ(state.level(2), 0);
  }

  return 0;
}
