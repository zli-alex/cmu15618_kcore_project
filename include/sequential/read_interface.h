#pragma once

#include <cstddef>

#include "sequential/descriptor_state.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"
#include "sequential/sequential_batch_engine.h"

namespace seq {

// Week 1 scaffold for CPLDS read-side semantics from:
// - Algorithm 3: check_DAG(desc)
// - Algorithm 4: read(v)
//
// This interface is intentionally shaped to match the future concurrent read path:
// 1) read batch number (b1)
// 2) read live level (l1)
// 3) read descriptor data (old_level, parent)
// 4) run check_DAG / root traversal
// 5) read live level again (l2)
// 6) read batch number again (b2)
// 7) return old_level estimate if MARKED else live-level estimate
//
// In Week 1, updates are processed sequentially and reads observe finalized state,
// so check_DAG always returns UNMARKED and reads return live-level estimates.
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

// Week 1 root traversal scaffold (future Algorithm 3 helper path).
VertexId find_root(const DescriptorState& descriptors, VertexId v);

// Week 1 check_DAG scaffold. Always UNMARKED for finalized-state reads.
DagMarkStatus check_dag(const DescriptorState& descriptors, VertexId v);

// Read-facing API for coreness estimate of a vertex.
double read_estimate(const LDSConfig& config, const LDSState& state,
                     const DescriptorState& descriptors, VertexId v,
                     ReadDebugInfo* debug = nullptr);

// Convenience overload for caller code that has the engine object.
double read_estimate(const SequentialBatchEngine& engine, VertexId v,
                     ReadDebugInfo* debug = nullptr);

}  // namespace seq
