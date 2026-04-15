#include <algorithm>
#include <vector>

#include "sequential/descriptor_state.h"

#include "test_common.h"

namespace {

seq::VertexId find_root(const seq::DescriptorState& d, seq::VertexId v) {
  if (v >= d.parent.size()) {
    return v;
  }
  seq::VertexId cur = v;
  for (std::size_t steps = 0; steps < d.parent.size(); ++steps) {
    const seq::VertexId p = d.parent[cur];
    if (p >= d.parent.size() || p == cur) {
      return cur;
    }
    cur = p;
  }
  return cur;
}

}  // namespace

int main() {
  seq::DescriptorState d(6);
  d.begin_batch(std::vector<int>({7, 7, 7, 7, 7, 7}));

  d.on_level_mutation_start(2, 5);
  d.on_level_mutation_start(1, 4);
  d.on_level_mutation_start(3, 3);
  d.on_level_mutation_start(1, 1);  // ignored for old_level; already recorded.

  EXPECT_EQ(d.old_level[2], 5);
  EXPECT_EQ(d.old_level[1], 4);
  EXPECT_EQ(d.old_level[3], 3);
  EXPECT_EQ(d.first_mutation_recorded[2], static_cast<std::uint8_t>(1));
  EXPECT_EQ(d.first_mutation_recorded[1], static_cast<std::uint8_t>(1));
  EXPECT_EQ(d.first_mutation_recorded[3], static_cast<std::uint8_t>(1));
  EXPECT_EQ(d.marked[2], static_cast<std::uint8_t>(1));
  EXPECT_EQ(d.marked[1], static_cast<std::uint8_t>(1));
  EXPECT_EQ(d.marked[3], static_cast<std::uint8_t>(1));

  std::vector<seq::VertexId> marked_vertices = {1, 2, 3};
  std::vector<seq::VertexId> roots;
  std::vector<seq::VertexId> non_roots;
  for (seq::VertexId v : marked_vertices) {
    if (find_root(d, v) == v) {
      roots.push_back(v);
    } else {
      non_roots.push_back(v);
    }
  }
  EXPECT_TRUE(!roots.empty());
  EXPECT_TRUE(!non_roots.empty());

  d.unmark_all_roots_first();
  EXPECT_EQ(d.last_unmark_order.size(), marked_vertices.size());

  for (seq::VertexId v : marked_vertices) {
    EXPECT_EQ(d.marked[v], static_cast<std::uint8_t>(0));
  }

  std::size_t first_non_root_pos = d.last_unmark_order.size();
  for (std::size_t i = 0; i < d.last_unmark_order.size(); ++i) {
    if (std::find(non_roots.begin(), non_roots.end(), d.last_unmark_order[i]) !=
        non_roots.end()) {
      first_non_root_pos = i;
      break;
    }
  }

  for (seq::VertexId r : roots) {
    auto it = std::find(d.last_unmark_order.begin(), d.last_unmark_order.end(), r);
    EXPECT_TRUE(it != d.last_unmark_order.end());
    const std::size_t pos = static_cast<std::size_t>(std::distance(d.last_unmark_order.begin(), it));
    EXPECT_TRUE(pos < first_non_root_pos);
  }

  return 0;
}
