//===----------------------------------------------------------------------===//
//
//                               Ranked
//
//===----------------------------------------------------------------------===//
//
//  test.cpp - 01/01/2022
//
//  Copyright (C) 2022. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <thread>

#include "ranked.h"

bool test_api_inc(ranked::RankedContext context) {
  std::string table("test-table");
  std::string key("test-key-inc");
  int number = 1;

  context.inc(table, key, number);

  return *context.get(table, key) == number;
}

bool test_api_incp(ranked::RankedContext context) {
  std::string table("test-table");
  std::string key("test-key-incp");
  int number = 1;
  int remain = 1;

  context.incp(table, key, number, remain);

  std::this_thread::sleep_for(std::chrono::milliseconds(3000));

  return *context.get(table, key) == 0;
}

void test(std::string name, bool succ) {
  std::cout << name << ": "
            << (succ ? "\x1B[32msuccess\033[0m" : "\x1B[31mfail\033[0m")
            << '\n';
}

int main() {
  ranked::RankedContext Instance;

  std::cout << "ranked test\n";

  test("inc", test_api_inc(Instance));
  test("incp", test_api_incp(Instance));
}