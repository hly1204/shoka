#include "fast_io.h"

#include <bits/stdc++.h>

#include <catch2/catch_all.hpp>

TEST_CASE("fast_io") {
  auto tmpf = std::tmpfile();
  std::fprintf(
      tmpf,
      "-2147483647 0 2147483647\n-9223372036854775807 0 9223372036854775807\n");
  for (int i = 0; i < 100; ++i) {
    std::fprintf(tmpf, "%d\n", i);
  }
  std::rewind(tmpf);

  FastIO io{tmpf};

  REQUIRE(io.read() == -2147483647);
  REQUIRE(io.read() == 0);
  REQUIRE(io.read() == 2147483647);
  REQUIRE(io.read((long long){}) == -9223372036854775807);
  REQUIRE(io.read((long long){}) == 0);
  REQUIRE(io.read((long long){}) == 9223372036854775807);

  REQUIRE(io.read(std::tuple<int, int>{}) == std::tuple<int, int>{0, 1});
  REQUIRE(io.read(std::vector<int>(3)) == std::vector<int>{2, 3, 4});
  REQUIRE(io.read(std::vector<std::vector<int>>(2, std::vector<int>(3))) ==
          std::vector<std::vector<int>>{{5, 6, 7}, {8, 9, 10}});
  REQUIRE(io.read_t<int, int, int>() == std::tuple<int, int, int>{11, 12, 13});
  REQUIRE(io.read_v(2) == std::vector<int>{14, 15});

  std::fclose(tmpf);
}