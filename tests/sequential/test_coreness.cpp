#include "sequential/coreness.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

#include "test_common.h"

int main() {
  seq::LDSConfig cfg;
  cfg.delta = 0.2;
  cfg.group_size = 2;
  cfg.num_levels = 500;

  seq::LDSState state(1000, cfg, 0);
  state.set_level(1, 350);

  EXPECT_NEAR(seq::estimate_coreness_from_level(cfg, 0, state.num_vertices()), 1.0,
              1e-9);
  EXPECT_NEAR(seq::estimate_coreness_from_level(cfg, 350, state.num_vertices()),
              1.2, 1e-9);

  auto all = seq::estimate_all_coreness(cfg, state);
  EXPECT_EQ(all.size(), static_cast<std::size_t>(1000));
  EXPECT_NEAR(all[1], 1.2, 1e-9);
  return 0;
}
