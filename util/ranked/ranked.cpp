//===----------------------------------------------------------------------===//
//
//                               Ranked
//
//===----------------------------------------------------------------------===//
//
//  ranked.cpp - 01/01/2022
//
//  Copyright (C) 2022. rollrat. All Rights Reserved.
//
//===----------------------------------------------------------------------===//

#include <iostream>

#include "ranked.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "ranked v1.0.0\n";
    std::cout << argv[0] << " <host> <port>";
    return 0;
  }

  ranked::RankedServer server;
  server.startServer(argv[1], std::atoi(argv[2]));
}