#include "core/batch_update.h"
#include "sequential/deletion_phase.h"
#include "sequential/descriptor_state.h"
#include "core/graph.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

#include "test_common.h"

int main() {
  seq::LDSConfig cfg;
  cfg.num_levels = 8;
  cfg.group_size = 2;
  cfg.delta = 0.2;
  cfg.lambda = 9.0;

  seq::Graph g(3);
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(0, 2);

  seq::LDSState s(3, cfg, 4);
  seq::DescriptorState d(3);

  std::vector<seq::EdgeUpdate> dels = {{seq::UpdateType::Delete, 0, 2}};
  std::size_t demotions = 0;
  std::size_t fixups = 0;
  seq::apply_deletions(g, s, d, cfg, dels, &demotions, &fixups);

  EXPECT_TRUE(demotions > 0);
  EXPECT_TRUE(s.level(0) <= 3);
  EXPECT_TRUE(s.level(2) <= 3);
  return 0;
}
