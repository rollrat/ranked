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

#include "ChronoTimer.hpp"
#include "ranked.hpp"

bool test_api_inc(ranked::RankedContext context) {
  std::string table("test-table");
  std::string key("test-key-inc");
  int number = 1;

  context.zincrby(table, number, key);

  return *context.zget(table, key) == number;
}

bool test_api_incp(ranked::RankedContext context) {
  std::string table("test-table");
  std::string key("test-key-incp");
  int number = 1;
  int remain = 1;

  context.zincrbyp(table, number, key, remain);

  std::this_thread::sleep_for(std::chrono::milliseconds(3000));

  return *context.zget(table, key) == 0;
}

bool test_api_incp_bulk(ranked::RankedContext context) {
  std::string table("test-table");
  std::string key("test-key-incp");
  int number = 1;
  int remain = 1;
  const int count = 1000000;

  for (int i = 0; i < count; i++) {
    context.zincrbyp(table, number, key, remain);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(3000));

  return *context.zget(table, key) == 0;
}

bool test_api_zrange(ranked::RankedContext context) {
  std::string table("test-zrange-table");
  std::string key_prefix("test-key-");
  const int count = 1000000;

  for (int i = 0; i < count; i++) {
    context.zincrby(table, i % 10, key_prefix + std::to_string(i % 100000));
  }

  for (int i = 0; i < 100; i++) {
    ofw::ChronoTimer ct;
    ct.start();
    auto result = context.zrange_withscores(table, 0, 0);
    ct.finish();

    for (auto v : result) {
      std::cout << v.first << ": " << v.second << '\n';
    }

    std::cout << "Time: " << *ct << '\n';
  }

  return true;
}

class NullBuffer : public std::streambuf {
public:
  int overflow(int c) { return c; }
};

bool test_command() {
  ranked::RankedCommandProcessor processor;

  NullBuffer null_buffer;
  std::ostream null_stream(&null_buffer);

  processor.processReceivePostLine("zadd table 1 mem", null_stream);
  processor.processReceivePostLine("zincrby table 1 mem", null_stream);
  processor.processReceivePostLine("zincrbyp table 1 mem 4", null_stream);

  for (int i = 0; i < 100000; i++) {
    processor.processReceivePostLine("zincrbyp table 1 mem " +
                                         std::to_string((i % 2 == 0) ? 1 : 7),
                                     null_stream);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(4000));

  ofw::ChronoTimer ct;
  ct.start();
  processor.processReceivePostLine("zget table mem", std::cout);
  ct.finish();
  std::cout << "Time: " << *ct << '\n';

  return true;
}

void test(std::string name, bool succ) {
  std::cout << name << ": "
            << (succ ? "\x1B[32msuccess\033[0m" : "\x1B[31mfail\033[0m")
            << '\n';
}

int main() {
  // ranked::RankedContext Instance;

  std::cout << "ranked test\n";

  // test("inc", test_api_inc(Instance));
  // test("incp", test_api_incp(Instance));
  // test("inc-bulk", test_api_zrange(Instance));
  test("command", test_command());
}