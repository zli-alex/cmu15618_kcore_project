#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc > 1 && std::string(argv[1]) == "--test") {
    std::cout << "seq_runner test placeholder: PASS\n";
    return 0;
  }

  std::cout << "seq_runner placeholder (Week 1 scaffold)\n";
  return 0;
}
