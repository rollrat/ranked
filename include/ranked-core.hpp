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
#include <memory>
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
  std::string member;
  int increment;

  RankedPeriodicUnit(int64_t expireTime, std::string member, int increment)
      : expireTime(expireTime), member(member), increment(increment) {}

  bool operator<(const RankedPeriodicUnit &other) const {
    return expireTime < other.expireTime;
  }
};

class RankedTable {
public:
  ~RankedTable() {
    map.clear();
    set.clear();
    std::priority_queue<
        std::shared_ptr<RankedPeriodicUnit>,
        std::vector<std::shared_ptr<RankedPeriodicUnit>>, compare>()
        .swap(minHeap);
  }

  void zadd(int increment, const std::string &member) {
    set_erase(member);
    map[member] = increment;
    set_insert(member);
  }

  Box<int> zincrby(int increment, const std::string &member) {
    if (!existsmember(member)) {
      map[member] = 0;
    } else {
      set_erase(member);
    }
    int result = map[member] = map[member] + increment;
    set_insert(member);
    return result;
  }
  Box<int> zincrbyp(int increment, const std::string &member, int remain) {
    int result = *zincrby(increment, member);
    minHeap.push(std::make_shared<RankedPeriodicUnit>(getTimeStamp() + remain,
                                                      member, -increment));
    return result;
  }

  std::vector<std::string> zrange(int offset, int count) {
    processPeriodic();

    std::set<std::pair<int, std::string>>::iterator iter = set.begin();

    for (; offset--;)
      iter++;

    if (count == -1) {
      count = map.size();
    }

    std::vector<std::string> result;
    result.reserve(count);

    for (; iter != set.end() && count; iter++, count--)
      result.emplace_back(iter->second);

    return result;
  }

  std::vector<std::pair<std::string, int>> zrange_withscores(int offset,
                                                             int count) {
    processPeriodic();

    std::set<std::pair<int, std::string>>::iterator iter = set.begin();

    for (; offset--;)
      iter++;

    if (count == -1) {
      count = map.size();
    }

    std::vector<std::pair<std::string, int>> result;
    result.reserve(count);

    for (; iter != set.end() && count; iter++, count--)
      result.push_back({iter->second, iter->first});

    return result;
  }

  std::vector<std::string> zrevrange(int offset, int count) {
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

  std::vector<std::pair<std::string, int>> zrevrange_withscores(int offset,
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

  Box<int> zget(const std::string &member) {
    processPeriodic();

    auto kv = map.find(member);
    if (kv != map.end()) {
      return Box<int>(kv->second);
    }
    return Box<int>();
  }

private:
  struct compare {
    bool operator()(const std::shared_ptr<RankedPeriodicUnit> &wp1,
                    const std::shared_ptr<RankedPeriodicUnit> &wp2) const {
      return *wp1 < *wp2;
    }
  };

  std::map<std::string, int> map;
  std::set<std::pair<int, std::string>> set;
  std::priority_queue<std::shared_ptr<RankedPeriodicUnit>,
                      std::vector<std::shared_ptr<RankedPeriodicUnit>>, compare>
      minHeap;

  int64_t getTimeStamp() {
    const auto p1 = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::seconds>(
               p1.time_since_epoch())
        .count();
  }

  void processPeriodic() {
    auto now = getTimeStamp();

    while (!minHeap.empty() && minHeap.top()->expireTime < now) {
      set_erase(minHeap.top()->member);
      map[minHeap.top()->member] -= minHeap.top()->increment;
      set_insert(minHeap.top()->member);
      minHeap.pop();
    }
  }

  void set_erase(const std::string &member) {
    set.erase(set.find({map[member], member}));
  }
  void set_insert(const std::string &member) {
    set.insert({map[member], member});
  }

  bool existsmember(const std::string &member) {
    return map.find(member) != map.end();
  }
};

class RankedContext {
public:
  void zadd(const std::string &tableName, int value,
            const std::string &member) {
    getTable(tableName)->zadd(value, member);
  }

  Box<int> zincrby(const std::string &tableName, int increment,
                   const std::string &member) {
    return getTable(tableName)->zincrby(increment, member);
  }
  Box<int> zincrbyp(const std::string &tableName, int increment,
                    const std::string &member, int remain) {
    return getTable(tableName)->zincrbyp(increment, member, remain);
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

  std::vector<std::string> zrevrange(const std::string &tableName, int offset,
                                     int count) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      return table->zrevrange(offset, count);
    }
    return std::vector<std::string>();
  }

  std::vector<std::pair<std::string, int>>
  zrevrange_withscores(const std::string &tableName, int offset, int count) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      return table->zrevrange_withscores(offset, count);
    }
    return std::vector<std::pair<std::string, int>>();
  }

  Box<int> zget(const std::string &tableName, const std::string &member) {
    auto table = getTable(tableName);
    if (table != nullptr) {
      return table->zget(member);
    }
    return Box<int>();
  }

  void flushall() { tableMap.clear(); }

private:
  std::map<std::string, std::shared_ptr<RankedTable>> tableMap;

  std::shared_ptr<RankedTable> getTable(const std::string &tableName) {
    auto table = tableMap.find(tableName);
    if (table != tableMap.end())
      return table->second;
    tableMap[tableName] = std::make_shared<RankedTable>();
    return tableMap[tableName];
  }
};

} // namespace ranked

#endif