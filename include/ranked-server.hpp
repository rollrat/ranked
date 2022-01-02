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

class RankedServer {
  httplib::Server svr;
  RankedContext context;

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

    std::istringstream oss(body);
    std::string line;
    while (std::getline(oss, line)) {
      line = StringUtil::trim(line);
      if (line.empty())
        continue;

      processReceivePostLine(line);
    }

    res.set_content("", "text/plain");
  }

  void processReceivePostLine(std::string &line) {
    std::istringstream ossl(line);
    std::string word;

    ossl >> word;

    if (word == "flushall") {
      do_flushall();
    }
  }

  void do_flushall() { context.flushall(); }
};

} // namespace ranked

#endif