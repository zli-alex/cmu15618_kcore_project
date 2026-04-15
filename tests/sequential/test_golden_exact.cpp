#include <vector>

#include "core/batch_update.h"
#include "sequential/sequential_batch_engine.h"

#include "test_common.h"

int main() {
  seq::LDSConfig cfg;
  cfg.num_levels = 8;
  cfg.group_size = 2;
  cfg.lambda = 9.0;
  cfg.delta = 0.2;

  seq::Graph g(3);
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(0, 2);

  seq::LDSState s(3, cfg, 2);
  seq::DescriptorState d(3);
  seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

  seq::BatchUpdate batch{{{seq::UpdateType::Delete, 0, 2}}};
  const auto result = seq::process_batch(engine, batch);

  EXPECT_VEC_EQ(result.levels_before, std::vector<int>({2, 2, 2}));
  EXPECT_VEC_EQ(result.levels_after, std::vector<int>({2, 2, 2}));
  EXPECT_VEC_EQ_UNORDERED(result.moved_vertices, std::vector<seq::VertexId>{});
  EXPECT_VEC_NEAR(result.coreness_estimate, std::vector<double>({1.0, 1.0, 1.0}),
                  1e-9);

  EXPECT_EQ(engine.descriptors.old_level[0], 2);
  EXPECT_EQ(engine.descriptors.old_level[1], 2);
  EXPECT_EQ(engine.descriptors.old_level[2], 2);

  return 0;
}
