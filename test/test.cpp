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

  ranked::instance.inc(table, key, number);

  return ranked::instance.get(table, key) == number;
}

void test_api_incp(std::string table, std::string key, int number, int remain) {
  std::string table("test-table");
  std::string key("test-key");
  int number = 1;

  ranked::instance.incp(table, key, number);

  return ranked::instance.get(table, key) == number;
}

int main() { std::cout << "test hello"; }