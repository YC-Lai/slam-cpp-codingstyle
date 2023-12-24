#include <iostream>

class A {
 public:
  A() { std::cout << getName(); }

  virtual const char* getName() { return "A"; }
};

class B : public A {
 public:
  virtual const char* getName() { return "B"; }
};

class C : public B {
 public:
  virtual const char* getName() { return "C"; }
};

int main() {
  C c;
  return 0;
}