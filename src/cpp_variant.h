#pragma once
#include <iostream>
#include <memory>

class TestA {
 public:
  TestA() = default;
  void Print() { std::cout << "TestA" << std::endl; }
};

class TestB {
 public:
  TestB() = default;
};

class Factory {
 public:
  static Factory *GetInstance() {
    static Factory instance;  // thread-safe
    return &instance;
  }

  auto GetTest() {
    struct result {
      operator std::shared_ptr<TestA>() {  // NOLINT
        return ins->GetA();
      }
      operator std::shared_ptr<TestB>() {  // NOLINT
        return ins->GetB();
      }
      Factory *ins;
    };
    return result{this};
  }

 protected:
  Factory() = default;
  std::shared_ptr<TestA> GetA() { return std::make_shared<TestA>(); }
  std::shared_ptr<TestB> GetB() { return std::make_shared<TestB>(); }
};

// std::shared_ptr<TestA> A = Factory::GetInstance()->GetTest();
// std::shared_ptr<TestB> B = Factory::GetInstance()->GetTest();