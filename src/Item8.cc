#include <iostream>

namespace {
int i = 20;
}

int main(int, char**) {
  std::cout << "Hello, from slam_cpp_codingstyle!\n";
  std::cout << "i: " << ::i << std::endl;
}
