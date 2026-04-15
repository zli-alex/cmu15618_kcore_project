#include "core/batch_update.h"
#include "sequential/descriptor_state.h"
#include "core/graph.h"
#include "sequential/insertion_phase.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

#include "test_common.h"

int main() {
  seq::LDSConfig cfg;
  cfg.num_levels = 8;
  cfg.group_size = 2;
  cfg.delta = 0.2;
  cfg.lambda = 9.0;

  seq::Graph g(4);
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  seq::LDSState s(4, cfg, 0);
  seq::DescriptorState d(4);

  std::vector<seq::EdgeUpdate> ins = {
      {seq::UpdateType::Insert, 1, 3},
  };

  std::size_t promotions = 0;
  std::size_t fixups = 0;
  seq::apply_insertions(g, s, d, cfg, ins, &promotions, &fixups);

  EXPECT_EQ(promotions, static_cast<std::size_t>(1));
  EXPECT_EQ(s.level(1), 1);
  EXPECT_EQ(s.level(0), 0);
  EXPECT_EQ(s.level(2), 0);
  EXPECT_EQ(s.level(3), 0);
  return 0;
}
