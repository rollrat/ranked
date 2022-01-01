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

bool test_api_inc() {
  std::string table("test-table");
  std::string key("test-key");
  int number = 1;

  ranked::RankedContext::Instance->inc(table, key, number);

  return **ranked::RankedContext::Instance->get(table, key) == number;
}

bool test_api_incp() {
  std::string table("test-table");
  std::string key("test-key");
  int number = 1;
  int remain = 5;

  ranked::RankedContext::Instance->incp(table, key, number, remain);

  return **ranked::RankedContext::Instance->get(table, key) == number;
}

void test(std::string name, bool succ) {
  std::cout << name << ": " << (succ ? "success" : "fail") << '\n';
}

int main() {
  ranked::RankedContext::InitInstance();

  test("inc", test_api_inc());
  test("incp", test_api_incp());
}