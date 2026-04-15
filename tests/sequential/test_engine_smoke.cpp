#include "core/batch_update.h"
#include "sequential/sequential_batch_engine.h"

#include "test_common.h"

int main() {
  seq::LDSConfig cfg;
  cfg.num_levels = 8;
  cfg.group_size = 2;
  cfg.delta = 0.2;
  cfg.lambda = 9.0;

  seq::Graph g(4);
  seq::LDSState s(4, cfg, 0);
  seq::DescriptorState d(4);
  seq::SequentialBatchEngine engine(std::move(g), cfg, std::move(s), std::move(d));

  seq::BatchUpdate b1{{
      {seq::UpdateType::Insert, 0, 1},
      {seq::UpdateType::Insert, 1, 2},
      {seq::UpdateType::Insert, 0, 2},
      {seq::UpdateType::Insert, 2, 3},
  }};
  auto r1 = seq::process_batch(engine, b1);

  EXPECT_EQ(engine.descriptors.batch_number, static_cast<std::size_t>(1));
  EXPECT_EQ(r1.coreness_estimate.size(), static_cast<std::size_t>(4));

  seq::BatchUpdate b2{{
      {seq::UpdateType::Delete, 0, 2},
      {seq::UpdateType::Delete, 1, 2},
  }};
  auto r2 = seq::process_batch(engine, b2);

  EXPECT_EQ(engine.descriptors.batch_number, static_cast<std::size_t>(2));
  EXPECT_TRUE(r2.num_demotions >= 0);
  return 0;
}
