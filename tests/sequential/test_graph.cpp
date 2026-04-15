#include "core/graph.h"

#include "test_common.h"

int main() {
  seq::Graph g(3);
  EXPECT_TRUE(g.add_edge(0, 1));
  EXPECT_TRUE(g.has_edge(0, 1));
  EXPECT_TRUE(g.has_edge(1, 0));
  EXPECT_TRUE(!g.add_edge(0, 1));
  EXPECT_TRUE(g.remove_edge(0, 1));
  EXPECT_TRUE(!g.has_edge(0, 1));
  EXPECT_TRUE(!g.remove_edge(0, 1));
  return 0;
}
