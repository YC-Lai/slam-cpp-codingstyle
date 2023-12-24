#include <iostream>
#include <type_traits>
#include <string>

class A {
 public:
  void DoSomething() {
    std::cout << "i: " << i << std::endl;
    std::cout << "j: " << j << std::endl;
  }

  int i = 0;
  float j = 2.f;
};

struct B {
 public:
  int i = 0;
  float j = 2.f;
};

struct C {
 public:
  std::string str;
};

int main(int, char**) {
  std::cout << "is_trivially_destructible:" << std::endl;
  std::cout << "A: " << std::is_trivially_destructible<A>::value << std::endl;
  std::cout << "B: " << std::is_trivially_destructible<B>::value << std::endl;
  std::cout << "C: " << std::is_trivially_destructible<C>::value << std::endl;
}