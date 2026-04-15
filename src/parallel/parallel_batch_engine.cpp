#include "parallel/parallel_batch_engine.h"

#include <utility>

namespace seq {
namespace parallel {

ParallelBatchEngine::ParallelBatchEngine(SequentialBatchEngine sequential_engine,
                                         ParallelOptions options)
    : sequential(std::move(sequential_engine)), options(options) {}

BatchResult process_batch(ParallelBatchEngine& engine, const BatchUpdate& batch) {
  [[maybe_unused]] const PartitionedUpdates partitioned =
      partition_insertions_first(batch);
  (void)partitioned;
  // Week 2 skeleton: correct semantics via the frozen oracle path.
  (void)engine.options.num_threads;
  return seq::process_batch(engine.sequential, batch);
}

}  // namespace parallel
}  // namespace seq
