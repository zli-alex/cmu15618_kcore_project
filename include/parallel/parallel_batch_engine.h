#pragma once

#include "core/batch_update.h"
#include "sequential/sequential_batch_engine.h"

namespace seq {
namespace parallel {

struct ParallelOptions {
  /// Intended for future OpenMP / task-parallel phases; must be >= 1.
  int num_threads = 1;
};

/// Parallel batch engine skeleton. Same batch input model as the sequential oracle;
/// insertion-then-deletion partitioning is explicit at this boundary.
struct ParallelBatchEngine {
  ParallelBatchEngine(SequentialBatchEngine sequential_engine, ParallelOptions options);

  SequentialBatchEngine sequential;
  ParallelOptions options;
};

BatchResult process_batch(ParallelBatchEngine& engine, const BatchUpdate& batch);

}  // namespace parallel
}  // namespace seq
