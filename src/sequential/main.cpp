#include <iostream>
#include <string>

#include "core/batch_update.h"
#include "sequential/sequential_batch_engine.h"

int main(int argc, char** argv) {
  using namespace seq;

  if (argc > 1 && std::string(argv[1]) == "--test") {
    std::cout << "Use `make test` to run sequential module tests.\n";
    return 0;
  }

  LDSConfig config;
  Graph graph(4);
  LDSState state(4, config, 0);
  DescriptorState descriptors(4);
  SequentialBatchEngine engine(std::move(graph), config, std::move(state),
                               std::move(descriptors));

  BatchUpdate batch{{
      {UpdateType::Insert, 0, 1},
      {UpdateType::Insert, 1, 2},
      {UpdateType::Insert, 0, 2},
  }};

  BatchResult result = process_batch(engine, batch);

  std::cout << "Processed batch " << engine.descriptors.batch_number << "\n";
  std::cout << "promotions=" << result.num_promotions
            << " demotions=" << result.num_demotions
            << " fixups=" << result.invariant_fixups << "\n";
  std::cout << "coreness:";
  for (double c : result.coreness_estimate) {
    std::cout << ' ' << c;
  }
  std::cout << "\n";

  return 0;
}
