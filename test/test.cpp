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

#include "ranked.h"

bool test_api_inc(ranked::RankedContext context) {
  std::string table("test-table");
  std::string key("test-key");
  int number = 1;

  std::cout << "m1\n";
  context.inc(table, key, number);

  return *context.get(table, key) == number;
}

bool test_api_incp(ranked::RankedContext context) {
  std::string table("test-table");
  std::string key("test-key");
  int number = 1;
  int remain = 5;

  context.incp(table, key, number, remain);

  return *context.get(table, key) == number;
}

void test(std::string name, bool succ) {
  std::cout << name << ": " << (succ ? "success" : "fail") << '\n';
}

int main() {
  ranked::RankedContext Instance;

  std::cout << "ranked test\n";

  test("inc", test_api_inc(Instance));
  test("incp", test_api_incp(Instance));
}