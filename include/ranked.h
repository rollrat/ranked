//===----------------------------------------------------------------------===//
//
//                               Ranked
//
//===----------------------------------------------------------------------===//
//
//  ranked.h - 01/01/2022
//
//  Copyright (C) 2022. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _RANKED_
#define _RANKED_

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <string>

namespace ranked {

template <typename T> class Box {
  T *ptr;

public:
  Box() : ptr(nullptr) {}
  Box(T value) { *ptr = value; }
  T operator*() const { return *ptr; }
};

class RankedItem {
public:
};

class RankedPeriodicUnit {
public:
  int64_t expireTime;
  std::string key;
  int number;

  RankedPeriodicUnit(int64_t expireTime, std::string key, int number)
      : expireTime(expireTime), key(key), number(number) {}

  bool operator<(const RankedPeriodicUnit &other) const {
    return expireTime < other.expireTime;
  }
};

class RankedTable {
public:
  void inc(const std::string &key, int number) {
    if (!existsKey(key)) {
      map[key] = number;
    } else {
      set.erase(set.find({map[key], key}));
      map[key] = map[key] + number;
    }
    set.insert({map[key], key});
  }

  void incp(const std::string &key, int number, int remain) {
    inc(key, number);
    minHeap.push(RankedPeriodicUnit(getTimeStamp() + remain, key, number));
  }

  std::vector<std::string> zrange(int offset, int count) {
    processPeriodic();

    std::vector<std::string> result;

    std::set<std::pair<int, std::string>>::iterator iter = set.begin();

    for (; offset--;)
      iter++;

    if (count == 0) {
      count = std::numeric_limits<int>::max();
    }

    for (; iter != set.end() && count > 0; iter++)
      result.emplace_back(iter->second);

    return result;
  }

  std::vector<std::pair<std::string, int>> zrange_withscores(int offset,
                                                             int count) {
    processPeriodic();

    std::vector<std::pair<std::string, int>> result;

    std::set<std::pair<int, std::string>>::iterator iter = set.begin();

    for (; offset--;)
      iter++;

    if (count == 0) {
      count = std::numeric_limits<int>::max();
    }

    for (; iter != set.end() && count > 0; iter++)
      result.push_back({iter->second, iter->first});

    return result;
  }

  Box<int> get(const std::string &key) {
    processPeriodic();

    auto kv = map.find(key);
    if (kv != map.end()) {
      return Box<int>(kv->second);
    }
    return Box<int>();
  }

private:
  std::map<std::string, int> map;
  std::set<std::pair<int, std::string>> set;
  std::priority_queue<RankedPeriodicUnit> minHeap;

  int64_t getTimeStamp() {
    const auto p1 = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::seconds>(
               p1.time_since_epoch())
        .count();
  }

  void processPeriodic() {
    auto now = getTimeStamp();

    while (!minHeap.empty() && minHeap.top().expireTime < now) {
      set.erase(set.find({map[minHeap.top().key], minHeap.top().key}));
      map[minHeap.top().key] -= minHeap.top().number;
      minHeap.pop();
      set.insert({map[minHeap.top().key], minHeap.top().key});
    }
  }

  bool existsKey(const std::string &key) { return map.find(key) != map.end(); }
};

class SortedMap {};

class RankedContext {
public:
  void inc(const std::string &tableName, const std::string &key, int number) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->inc(key, number);
    }
  }

  void incp(const std::string &tableName, const std::string &key, int number,
            int remain) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->incp(key, number, remain);
    }
  }

  std::vector<std::string> zrange(const std::string &tableName, int offset,
                                  int count) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      return table->zrange(offset, count);
    }
    return std::vector<std::string>();
  }

  std::vector<std::pair<std::string, int>>
  zrange_withscores(const std::string &tableName, int offset, int count) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      return table->zrange_withscores(offset, count);
    }
    return std::vector<std::pair<std::string, int>>();
  }

  Box<int> get(const std::string &tableName, const std::string &key) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      return table->get(key);
    }
    return Box<int>();
  }

private:
  std::map<std::string, RankedTable *> tableMap;

  RankedTable *getTable(const std::string &tableName) {
    auto table = tableMap.find(tableName);
    if (table != tableMap.end())
      return table->second;
    tableMap[tableName] = new RankedTable();
    return tableMap[tableName];
  }
};

} // namespace ranked

#endif