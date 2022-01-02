//===----------------------------------------------------------------------===//
//
//                               Ranked
//
//===----------------------------------------------------------------------===//
//
//  ranked-server.hpp - 01/02/2022
//
//  Copyright (C) 2022. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#ifndef _RANKED_SERVER_
#define _RANKED_SERVER_

#include <sstream>

#include "httplib.h"
#include "ranked-core.hpp"

namespace ranked {

class StringUtil {
public:
  static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

  // trim from end
  static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace)))
                .base(),
            s.end());
    return s;
  }

  // trim from both ends
  static inline std::string &trim(std::string &s) { return ltrim(rtrim(s)); }
};

class RankedCommandProcessor {
  RankedContext context;

public:
  void processReceivePost(std::istream &input, std::ostream &output) {
    std::string line;
    while (std::getline(input, line)) {
      line = StringUtil::trim(line);

      if (line.empty())
        continue;

      processReceivePostLine(line, output);
    }
  }

  void processReceivePostLine(const std::string &line, std::ostream &output) {
    std::istringstream iss(line);
    std::string word;

    iss >> word;

    if (word == "flushall") {
      do_flushall();
      return;
    }

    // Input format error!
    // Skip this command
    if (iss.eof()) {
      return;
    }

    std::string tableName;
    iss >> tableName;

    if (word == "zadd") {
      do_zadd(tableName, iss);
    } else if (word == "zincrby") {
      do_zincrby(tableName, iss, output);
    } else if (word == "zincrbyp") {
      do_zincrbyp(tableName, iss, output);
    } else if (word == "zrange") {
      do_zrange(tableName, iss, output);
    } else if (word == "zrevrange") {
      do_zrevrange(tableName, iss, output);
    } else if (word == "zget") {
      do_zget(tableName, iss, output);
    }
  }

private:
  // accept zincrby <table> [<increment> <member>]+
  void do_zadd(const std::string &tableName, std::istringstream &iss) {
    std::string member;
    int value;
    while (!iss.eof() && (iss >> value >> member)) {
      context.zadd(tableName, value, member);
    }
  }

  // accept zincrbyp <table> [<increment> <member>]+
  void do_zincrby(const std::string &tableName, std::istringstream &iss,
                  std::ostream &output) {
    std::string member;
    int increment;
    while (!iss.eof() && (iss >> increment >> member)) {
      auto result = context.zincrby(tableName, increment, member);
      if (!result.isNull())
        output << *result << '\n';
      else
        output << "null\n";
    }
  }

  // accept zincrbyp <table> [<increment> <member>]+ <expire>
  void do_zincrbyp(const std::string &tableName, std::istringstream &iss,
                   std::ostream &output) {
    std::string member;
    int increment, expire;
    while (!iss.eof() && (iss >> increment >> member >> expire)) {
      auto result = context.zincrbyp(tableName, increment, member, expire);
      if (!result.isNull())
        output << *result << '\n';
      else
        output << "null\n";
    }
  }

  // accept zrange <table> <offset> <count> [withscores]
  void do_zrange(const std::string &tableName, std::istringstream &iss,
                 std::ostream &output) {
    int offset, count;
    iss >> offset >> count;

    //
    // if option exsists
    //
    if (!iss.eof()) {

      std::string op;
      iss >> op;

      if (op == "withscores") {
        auto result = context.zrange_withscores(tableName, offset, count);

        for (auto kv : result)
          output << kv.first << ' ' << kv.second << '\n';

        return;
      }
    }

    //
    //  any options
    //
    auto result = context.zrange(tableName, offset, count);
    for (auto str : result)
      output << str << '\n';
  }

  // accept zrevrange <table> <offset> <count> [withscores]
  void do_zrevrange(const std::string &tableName, std::istringstream &iss,
                    std::ostream &output) {
    int offset, count;
    iss >> offset >> count;

    //
    // if option exsists
    //
    if (!iss.eof()) {

      std::string op;
      iss >> op;

      if (op == "withscores") {
        auto result = context.zrevrange_withscores(tableName, offset, count);

        for (auto kv : result)
          output << kv.first << ' ' << kv.second << '\n';

        return;
      }
    }

    //
    //  any options
    //
    auto result = context.zrevrange(tableName, offset, count);
    for (auto str : result)
      output << str << '\n';
  }

  // accept zget <table> [<member>]+
  void do_zget(const std::string &tableName, std::istringstream &iss,
               std::ostream &output) {
    std::string member;
    while (!iss.eof() && (iss >> member)) {
      auto result = context.zget(tableName, member);
      if (!result.isNull())
        output << *result << '\n';
      else
        output << "null\n";
    }
  }

  void do_flushall() { context.flushall(); }
};

class RankedServer {
  httplib::Server svr;
  RankedCommandProcessor processor;

public:
  RankedServer() {
    svr.set_payload_max_length(1024 * 1024 * 512); // 512 MB

    // Handle Post Data
    svr.Post("/", [&](const httplib::Request &req, httplib::Response &res) {
      receivePost(req, res);
    });
  }

  void startServer(const char *host, int port) { svr.listen(host, port); }

private:
  void receivePost(const httplib::Request &req, httplib::Response &res) {
    auto body = req.body;

    std::istringstream input(body);
    std::ostringstream output;

    processor.processReceivePost(input, output);

    res.set_content("", "text/plain");
  }
};

} // namespace ranked

#endif