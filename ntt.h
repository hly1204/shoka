#pragma once

#include "primitive_root.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <stdexcept>

template <typename Mod> struct NttT {
  static void assert_power_of_two(int n) {
    if (n & (n - 1)) {
      throw std::invalid_argument(std::to_string(n) + " is not a power of two");
    }
  }

  static constexpr int min_power_of_two(int n) {
    return n == 1 ? 1 : 1 << (32 - __builtin_clz(n - 1));
  }

  static constexpr Mod get_primitive_root() { return G; }

  void reserve(int n) {
    if (max_n < n) {
      assert_power_of_two(n);
      if ((Mod::mod() - 1) % n != 0) {
        throw std::invalid_argument(std::to_string(n) +
                                    " is not a divisor of (Mod::mod() - 1)");
      }
      max_n = n;
      twiddles.resize(max_n + 1);
      twiddles[0] = Mod{1};
      auto omega = binpow(G, (Mod::mod() - 1) / n);
      for (int i = 1; i <= max_n; ++i) {
        twiddles[i] = twiddles[i - 1] * omega;
      }
      for (int i = 0; i < NUMBER_OF_BUFFER; ++i) {
        buffers[i].reserve(max_n);
      }
    }
  }

  template <int i> Mod *raw_buffer() { return buffers[i].data(); }

  void dit(int n, Mod *a) {
    assert_power_of_two(n);
    for (int m = 1; m < n; m <<= 1) {
      auto step = max_n / (m << 1);
      for (int i = 0; i < n; i += m << 1) {
        int tid = 0;
        for (int r = i; r < i + m; ++r) {
          Mod tmp = twiddles[tid] * a[r + m];
          a[r + m] = a[r];
          a[r + m] -= tmp;
          a[r] += tmp;
          tid += step;
        }
      }
    }
  }

  void dif(int n, Mod *a) {
    assert_power_of_two(n);
    for (int m = n; m >>= 1;) {
      auto step = max_n / (m << 1);
      for (int i = 0; i < n; i += m << 1) {
        int tid = max_n;
        for (int r = i; r < i + m; ++r) {
          Mod tmp = a[r];
          tmp -= a[r + m];
          a[r] += a[r + m];
          a[r + m] = twiddles[tid] * tmp;
          tid -= step;
        }
      }
    }
  }

private:
  static constexpr int NUMBER_OF_BUFFER = 5;

  static constexpr Mod G = FiniteField<Mod>::primitive_root();

  int max_n = 0;
  std::vector<Mod> twiddles;
  std::array<std::vector<Mod>, NUMBER_OF_BUFFER> buffers;
};
