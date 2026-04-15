#include <vector>

#include "core/batch_update.h"
#include "sequential/deletion_phase.h"
#include "sequential/descriptor_state.h"
#include "core/graph.h"
#include "sequential/insertion_phase.h"
#include "sequential/invariants.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

#include "test_common.h"

int main() {
  seq::LDSConfig cfg;
  cfg.num_levels = 8;
  cfg.group_size = 1;
  cfg.delta = 0.2;
  cfg.lambda = 9.0;

  // 1) Invariant 2 boundary: l = 0 should always hold.
  {
    seq::Graph g(3);
    seq::LDSState s(3, cfg, 0);
    EXPECT_TRUE(seq::check_invariant2_vertex(g, cfg, s, 0));
    EXPECT_TRUE(seq::check_invariant2_vertex(g, cfg, s, 1));
    EXPECT_TRUE(seq::check_invariant2_vertex(g, cfg, s, 2));
  }

  // 2) Invariant 1 boundary: top level cannot promote beyond K-1.
  {
    seq::Graph g(6);
    for (seq::VertexId u = 0; u < 6; ++u) {
      for (seq::VertexId v = u + 1; v < 6; ++v) {
        g.add_edge(u, v);
      }
    }
    seq::LDSState s(6, cfg, cfg.num_levels - 1);
    EXPECT_TRUE(!seq::promote_vertex_one_level(g, cfg, s, 0));
    EXPECT_EQ(s.level(0), cfg.num_levels - 1);
  }

  // 3) Desire level can become 0.
  {
    seq::Graph g(2);
    seq::LDSState s(2, cfg, 3);
    const auto desire = seq::compute_desire_levels(g, cfg, s, {0});
    EXPECT_VEC_EQ(desire, std::vector<int>({0}));
  }

  // 4) Insertion can climb multiple levels in one insertion phase.
  {
    seq::Graph g(6);
    for (seq::VertexId u = 0; u < 6; ++u) {
      for (seq::VertexId v = u + 1; v < 6; ++v) {
        g.add_edge(u, v);
      }
    }
    seq::LDSState s(6, cfg, 0);
    seq::DescriptorState d(6);

    std::size_t promotions = 0;
    std::size_t fixups = 0;
    seq::BatchUpdate empty_batch;
    (void)empty_batch;
    seq::apply_insertions(g, s, d, cfg, {}, &promotions, &fixups);

    EXPECT_TRUE(promotions >= static_cast<std::size_t>(12));
    EXPECT_TRUE(s.level(0) >= 2);
  }

  return 0;
}
