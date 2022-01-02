//===----------------------------------------------------------------------===//
//
//                               Ranked
//
//===----------------------------------------------------------------------===//
//
//  ranked-core.h - 01/01/2022
//
//  Copyright (C) 2022. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _RANKED_CORE_
#define _RANKED_CORE_

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
  void zadd(const std::string &key, int number) {
    if (!existsKey(key)) {
      map[key] = number;
    } else {
      set_erase(key);
      map[key] = map[key] + number;
    }
    set_insert(key);
  }
  void zaddp(const std::string &key, int number, int remain) {
    zadd(key, number);
    minHeap.push(RankedPeriodicUnit(getTimeStamp() + remain, key, -number));
  }
  void zinc(const std::string &key) { zadd(key, 1); }
  void zincp(const std::string &key, int remain) {
    zadd(key, 1);
    minHeap.push(RankedPeriodicUnit(getTimeStamp() + remain, key, 1));
  }

  void zsub(const std::string &key, int number) { zadd(key, -number); }
  void zsubp(const std::string &key, int number, int remain) {
    zsubp(key, -number, remain);
  }
  void zdec(const std::string &key) { zadd(key, -1); }
  void zdecp(const std::string &key, int remain) {
    zadd(key, -1);
    minHeap.push(RankedPeriodicUnit(getTimeStamp() + remain, key, -1));
  }

  std::vector<std::string> zrange(int offset, int count) {
    processPeriodic();

    std::set<std::pair<int, std::string>>::reverse_iterator iter = set.rbegin();

    for (; offset--;)
      iter++;

    if (count == 0) {
      count = map.size();
    }

    std::vector<std::string> result;
    result.reserve(count);

    for (; iter != set.rend() && count; iter++, count--)
      result.emplace_back(iter->second);

    return result;
  }

  std::vector<std::pair<std::string, int>> zrange_withscores(int offset,
                                                             int count) {
    processPeriodic();

    std::set<std::pair<int, std::string>>::reverse_iterator iter = set.rbegin();

    for (; offset--;)
      iter++;

    if (count == 0) {
      count = map.size();
    }

    std::vector<std::pair<std::string, int>> result;
    result.reserve(count);

    for (; iter != set.rend() && count; iter++, count--)
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
      set_erase(minHeap.top().key);
      map[minHeap.top().key] -= minHeap.top().number;
      set_insert(minHeap.top().key);
      minHeap.pop();
    }
  }

  void set_erase(const std::string &key) {
    set.erase(set.find({map[key], key}));
  }
  void set_insert(const std::string &key) { set.insert({map[key], key}); }

  bool existsKey(const std::string &key) { return map.find(key) != map.end(); }
};

class RankedContext {
public:
  void zadd(const std::string &tableName, const std::string &key, int number) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->zadd(key, number);
    }
  }

  void zaddp(const std::string &tableName, const std::string &key, int number,
             int remain) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->zaddp(key, number, remain);
    }
  }

  void zinc(const std::string &tableName, const std::string &key) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->zinc(key);
    }
  }

  void zincp(const std::string &tableName, const std::string &key, int remain) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->zincp(key, remain);
    }
  }

  void zsub(const std::string &tableName, const std::string &key, int number) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->zsub(key, number);
    }
  }

  void zsubp(const std::string &tableName, const std::string &key, int number,
             int remain) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->zsubp(key, number, remain);
    }
  }

  void zdec(const std::string &tableName, const std::string &key) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->zdec(key);
    }
  }

  void zdecp(const std::string &tableName, const std::string &key, int remain) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      table->zdecp(key, remain);
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