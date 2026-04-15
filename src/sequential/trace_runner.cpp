#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "core/batch_update.h"
#include "sequential/sequential_batch_engine.h"

namespace {

struct Args {
  std::string graph_path;
  std::string batch_path;
  std::string out_path;
  int num_levels = 8;
  int group_size = 2;
  double delta = 0.2;
  double lambda = 9.0;
};

void print_usage() {
  std::cerr
      << "Usage: seq_trace_runner --graph <graph.txt> --batch <batch.txt> --out <out.json> "
      << "[--num-levels N] [--group-size G] [--delta D] [--lambda L]\n";
}

bool parse_args(int argc, char** argv, Args* args) {
  for (int i = 1; i < argc; ++i) {
    const std::string flag = argv[i];
    if (i + 1 >= argc) {
      print_usage();
      return false;
    }
    const std::string value = argv[++i];
    if (flag == "--graph") {
      args->graph_path = value;
    } else if (flag == "--batch") {
      args->batch_path = value;
    } else if (flag == "--out") {
      args->out_path = value;
    } else if (flag == "--num-levels") {
      args->num_levels = std::stoi(value);
    } else if (flag == "--group-size") {
      args->group_size = std::stoi(value);
    } else if (flag == "--delta") {
      args->delta = std::stod(value);
    } else if (flag == "--lambda") {
      args->lambda = std::stod(value);
    } else {
      std::cerr << "Unknown flag: " << flag << "\n";
      print_usage();
      return false;
    }
  }

  if (args->graph_path.empty() || args->batch_path.empty() || args->out_path.empty()) {
    print_usage();
    return false;
  }
  return true;
}

std::size_t detect_num_vertices(const std::string& graph_path,
                                const std::string& batch_path) {
  std::size_t max_vertex = 0;

  {
    std::ifstream in(graph_path);
    std::size_t u = 0;
    std::size_t v = 0;
    while (in >> u >> v) {
      max_vertex = std::max(max_vertex, std::max(u, v));
    }
  }

  {
    std::ifstream in(batch_path);
    char op = 'I';
    std::size_t u = 0;
    std::size_t v = 0;
    while (in >> op >> u >> v) {
      max_vertex = std::max(max_vertex, std::max(u, v));
    }
  }

  return max_vertex + 1;
}

seq::Graph load_graph(const std::string& path, std::size_t num_vertices) {
  seq::Graph graph(num_vertices);
  std::ifstream in(path);
  std::size_t u = 0;
  std::size_t v = 0;
  while (in >> u >> v) {
    graph.add_edge(u, v);
  }
  return graph;
}

seq::BatchUpdate load_batch(const std::string& path) {
  seq::BatchUpdate batch;
  std::ifstream in(path);
  char op = 'I';
  std::size_t u = 0;
  std::size_t v = 0;
  while (in >> op >> u >> v) {
    batch.updates.push_back(
        {op == 'I' ? seq::UpdateType::Insert : seq::UpdateType::Delete, u, v});
  }
  return batch;
}

void write_int_array(std::ofstream& out, const std::vector<int>& arr) {
  out << "[";
  for (std::size_t i = 0; i < arr.size(); ++i) {
    out << arr[i];
    if (i + 1 != arr.size()) {
      out << ", ";
    }
  }
  out << "]";
}

void write_double_array(std::ofstream& out, const std::vector<double>& arr) {
  out << "[";
  for (std::size_t i = 0; i < arr.size(); ++i) {
    out << arr[i];
    if (i + 1 != arr.size()) {
      out << ", ";
    }
  }
  out << "]";
}

void write_vertex_array(std::ofstream& out, const std::vector<seq::VertexId>& arr) {
  out << "[";
  for (std::size_t i = 0; i < arr.size(); ++i) {
    out << arr[i];
    if (i + 1 != arr.size()) {
      out << ", ";
    }
  }
  out << "]";
}

}  // namespace

int main(int argc, char** argv) {
  Args args;
  if (!parse_args(argc, argv, &args)) {
    return 1;
  }

  seq::LDSConfig config;
  config.num_levels = args.num_levels;
  config.group_size = args.group_size;
  config.delta = args.delta;
  config.lambda = args.lambda;

  const std::size_t n = detect_num_vertices(args.graph_path, args.batch_path);
  seq::Graph graph = load_graph(args.graph_path, n);
  seq::LDSState state(n, config, 0);
  seq::DescriptorState descriptors(n);
  seq::SequentialBatchEngine engine(std::move(graph), config, std::move(state),
                                    std::move(descriptors));

  // Stabilize initial graph state before applying the trace batch.
  seq::BatchUpdate empty_batch;
  (void)seq::process_batch(engine, empty_batch);

  const seq::BatchUpdate batch = load_batch(args.batch_path);
  const auto t0 = std::chrono::steady_clock::now();
  const seq::BatchResult result = seq::process_batch(engine, batch);
  const auto t1 = std::chrono::steady_clock::now();
  const double elapsed_ms =
      std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() /
      1000.0;

  std::ofstream out(args.out_path);
  if (!out) {
    std::cerr << "Failed to open output path: " << args.out_path << "\n";
    return 1;
  }

  out << "{\n";
  out << "  \"implementation\": \"sequential_week1\",\n";
  out << "  \"trace\": {\n";
  out << "    \"graph\": \"" << args.graph_path << "\",\n";
  out << "    \"batch\": \"" << args.batch_path << "\"\n";
  out << "  },\n";
  out << "  \"config\": {\n";
  out << "    \"num_levels\": " << config.num_levels << ",\n";
  out << "    \"group_size\": " << config.group_size << ",\n";
  out << "    \"delta\": " << config.delta << ",\n";
  out << "    \"lambda\": " << config.lambda << "\n";
  out << "  },\n";
  out << "  \"summary\": {\n";
  out << "    \"elapsed_ms\": " << elapsed_ms << ",\n";
  out << "    \"promotions\": " << result.num_promotions << ",\n";
  out << "    \"demotions\": " << result.num_demotions << ",\n";
  out << "    \"invariant_fixups\": " << result.invariant_fixups << "\n";
  out << "  },\n";
  out << "  \"levels\": {\n";
  out << "    \"before\": ";
  write_int_array(out, result.levels_before);
  out << ",\n";
  out << "    \"after\": ";
  write_int_array(out, result.levels_after);
  out << "\n";
  out << "  },\n";
  out << "  \"coreness_estimate\": ";
  write_double_array(out, result.coreness_estimate);
  out << ",\n";
  out << "  \"moved_vertices\": ";
  write_vertex_array(out, result.moved_vertices);
  out << ",\n";
  out << "  \"descriptor\": {\n";
  out << "    \"batch_number\": " << engine.descriptors.batch_number << ",\n";
  out << "    \"old_level\": ";
  write_int_array(out, engine.descriptors.old_level);
  out << "\n";
  out << "  }\n";
  out << "}\n";

  std::cout << "Wrote normalized trace output: " << args.out_path << "\n";
  return 0;
}
