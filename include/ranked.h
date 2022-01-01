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

#include <map>
#include <queue>
#include <string>

namespace ranked {

template <typename T> class Box {
  T *ptr;

public:
  Box() : ptr(nullptr) {}
  Box(T value) { *ptr = value; }
  T &&operator*() const { return *ptr; }
};

class RankedItem {
public:
};

class RankedTable {
public:
  void inc(const std::string &key, int number) {
    if (!existsKey(key)) {
      map[key] = number;
    } else {
      map[key] = map[key] + number;
    }
  }

  void incp(const std::string &key, int number, int remain);

  Box<int> *get(const std::string &key) const {
    auto kv = map.find(key);
    if (kv != map.end()) {
      return new Box<int>(kv->second);
    }
    return new Box<int>();
  }

private:
  std::map<std::string, int> map;

  bool existsKey(const std::string &key) { return map.find(key) != map.end(); }
};

class SortedMap {};

class RankedContext {
public:
  static RankedContext *Instance;

  static void InitInstance() { Instance = new RankedContext(); }

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

  Box<int> *get(const std::string &tableName, const std::string &key) const {
    auto table = getTable(tableName);
    if (table != nullptr) {
      return table->get(key);
    }
    return new Box<int>();
  }

private:
  std::map<std::string, RankedTable *> tableMap;

  RankedTable *getTable(const std::string &tableName) const {
    auto table = tableMap.find(tableName);
    if (table != tableMap.end())
      return table->second;
    return nullptr;
  }
};

} // namespace ranked

#endif