#include <vector>

#include "core/batch_update.h"
#include "sequential/deletion_phase.h"
#include "core/graph.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

#include "test_common.h"

int main() {
  seq::LDSConfig cfg;
  cfg.num_levels = 8;
  cfg.group_size = 2;
  cfg.lambda = 9.0;
  cfg.delta = 0.2;

  // Case 1: no move needed.
  {
    seq::Graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(0, 2);
    seq::LDSState s(3, cfg, 2);
    const auto desire = seq::compute_desire_levels(g, cfg, s, {0});
    EXPECT_VEC_EQ(desire, std::vector<int>({2}));
  }

  // Case 2: drop by one level.
  {
    seq::Graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    seq::LDSState s(3, cfg, 2);
    const auto desire = seq::compute_desire_levels(g, cfg, s, {0});
    EXPECT_VEC_EQ(desire, std::vector<int>({2}));
  }

  // Case 3: drop by multiple levels.
  {
    seq::Graph g(4);
    seq::LDSState s(4, cfg, 3);
    const auto desire = seq::compute_desire_levels(g, cfg, s, {0});
    EXPECT_VEC_EQ(desire, std::vector<int>({0}));
  }

  // Case 4: monotonicity after processing low level.
  {
    seq::Graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    seq::LDSState s(3, cfg, 0);
    s.set_level(1, 2);
    s.set_level(2, 2);
    const auto desire = seq::compute_desire_levels(g, cfg, s, {1, 2});
    const int processed_level = 0;
    for (int d : desire) {
      EXPECT_TRUE(d > processed_level);
    }
  }

  return 0;
}
