#include "../ntt.h"
#include "../mod.h"

#include "gtest/gtest.h"

#include <random>

static const int MOD = 998244353;

using ModT = montgomery::Montgomery32T<MOD>;

std::mt19937_64 mt;

std::vector<ModT> random_poly(int n) {
  std::vector<ModT> result(n);
  for (int i = 0; i < n; ++i) {
    result[i] = ModT(mt() % MOD);
  }
  return result;
}

std::vector<ModT> random_poly1(int n) {
  std::vector<ModT> result(n);
  result[0] = ModT(1 + mt() % (MOD - 1));
  for (int i = 1; i < n; ++i) {
    result[i] = ModT(mt() % MOD);
  }
  return result;
}

static const int n = 1 << 10;

ntt::Poly<ntt::NTT<ModT>> poly(n);

TEST(NTT, Multiplication) {
  std::vector<ModT> f = random_poly((n >> 1) - 1);
  std::vector<ModT> g = random_poly((n >> 1) - 2);
  std::vector<ModT> fg;
  poly.multiply(fg, f, g);

  std::vector<ModT> answer(f.size() + g.size() - 1);
  for (int i = 0; i < f.size(); ++i) {
    for (int j = 0; j < g.size(); ++j) {
      answer[i + j] += f[i] * g[j];
    }
  }
  ASSERT_EQ(fg.size(), answer.size());
  for (int i = 0; i < answer.size(); ++i) {
    ASSERT_EQ(fg[i].get(), answer[i].get());
  }
}

TEST(NTT, Inversion) {
  std::vector<ModT> f = random_poly1(n);
  std::vector<ModT> f_inv(n);
  poly.inverse(n, f_inv.data(), f.data());

  std::vector<ModT> answer(n);
  answer[0] = f[0].inverse();
  for (int i = 1; i < n; ++i) {
    answer[i] = ModT(0);
    for (int j = 0; j < i; ++j) {
      answer[i] -= f[i - j] * answer[j];
    }
    answer[i] *= answer[0];
  }
  for (int i = 0; i < n; ++i) {
    ASSERT_EQ(f_inv[i].get(), answer[i].get());
  }
}
