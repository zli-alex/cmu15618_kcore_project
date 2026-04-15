#pragma once

#include <cstddef>

#include "sequential/descriptor_state.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"
#include "sequential/sequential_batch_engine.h"

namespace seq {

// CPLDS read-side scaffold (Algorithm 3 / 4 shape; lock-free retries deferred).
//
// Intended future concurrent read sequence (batch sandwich + retries) is documented
// below; the current implementation uses a single snapshot for the return value.
//
// Snapshot behavior today:
// 1) read batch number (b1) — recorded in ReadDebugInfo only; does not affect return value
// 2) read live level (l1)
// 3) read descriptor data (old_level[v], parent)
// 4) find root via parent pointers; check_dag(v) is Marked iff marked[root]
// 5) read live level again (l2) — ReadDebugInfo only
// 6) read batch number again (b2) — ReadDebugInfo only
// 7) Return value: if Marked, coreness from old_level[v] (per-vertex); else live state.level(v).
//    Batch-number sandwich / retries are not used to alter returns yet.
//
// Finalized post-batch: update path clears all marked bits, so check_dag is always
// Unmarked and read_estimate matches live-level coreness.
enum class DagMarkStatus { Marked, Unmarked };

struct ReadDebugInfo {
  std::size_t batch_before = 0;
  std::size_t batch_after = 0;
  int live_level_first = 0;
  int live_level_second = 0;
  int descriptor_old_level = 0;
  VertexId root = 0;
  DagMarkStatus dag_status = DagMarkStatus::Unmarked;
};

// Follow parent pointers until a self-loop or invalid parent; returns the root id.
VertexId find_root(const DescriptorState& descriptors, VertexId v);

// Marked iff the root of v's parent component has marked[root] set.
DagMarkStatus check_dag(const DescriptorState& descriptors, VertexId v);

// Coreness estimate for v: old_level[v] when DAG root is marked, else live level.
double read_estimate(const LDSConfig& config, const LDSState& state,
                     const DescriptorState& descriptors, VertexId v,
                     ReadDebugInfo* debug = nullptr);

// Convenience overload for caller code that has the engine object.
double read_estimate(const SequentialBatchEngine& engine, VertexId v,
                     ReadDebugInfo* debug = nullptr);

}  // namespace seq
