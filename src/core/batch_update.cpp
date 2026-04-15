#include "core/batch_update.h"

namespace seq {

PartitionedUpdates partition_insertions_first(const BatchUpdate& batch) {
  PartitionedUpdates out;
  out.insertions.reserve(batch.updates.size());
  out.deletions.reserve(batch.updates.size());
  for (const auto& update : batch.updates) {
    if (update.type == UpdateType::Insert) {
      out.insertions.push_back(update);
    } else {
      out.deletions.push_back(update);
    }
  }
  return out;
}

}  // namespace seq
