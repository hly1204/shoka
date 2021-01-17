#include "../montgomery.h"

#include <random>

#include "gtest/gtest.h"

static const int MOD = 998244353;

TEST(Montgomery, Test) {
  using ModT = montgomery::Montgomery32T<MOD>;

  std::mt19937_64 gen(0);
  uint32_t a = gen() % MOD;
  uint32_t b = gen() % MOD;
  ModT ma(a);
  ASSERT_EQ(ma.get(), a);
  ModT mb(b);
  ASSERT_EQ(mb.get(), b);
  ASSERT_EQ((ma + mb).get(), (a + b) % MOD);
  ASSERT_EQ((ma * mb).get(), (uint64_t)a * b % MOD);
}