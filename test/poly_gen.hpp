#include "poly_gen.h"

#include "mod.h"

#include <bits/stdc++.h>

#include <catch2/catch_all.hpp>
#include <catch2/generators/catch_generators.hpp>

template <int Index> using C = dsl::Val<Index>;

namespace dsl {
template <typename P> struct CustomOp {
  template <typename Ctx>
  struct StoreT : public CacheBaseT<Ctx, StoreT>, public UnaryOpStoreT<Ctx, P> {
    using CacheBaseT<Ctx, StoreT>::cache;
    using UnaryOp = UnaryOpStoreT<Ctx, P>;
    using UnaryOp::p;

    explicit StoreT(Ctx &ctx)
        : UnaryOp{ctx}, min_deg{p.min_deg}, max_deg{INT_MAX} {}

    void compute_next() {
      int k = cache.size();
      cache.push_back((k ? (*this)[k - 1] : Ctx::ZERO) + p[k]);
    }

    const int min_deg, max_deg;
  };
};
} // namespace dsl

TEST_CASE("poly_gen") {
  using Mod = ModT<998'244'353>;
  using Vector = std::vector<Mod>;

  using namespace dsl;

  auto take = [&](auto &f, int n) {
    Vector p(n);
    for (int i = 0; i < n; ++i) {
      p[i] = f[i];
    }
    return p;
  };

  SECTION("c") {
    using Ctx = PolyCtxT<Mod, 1, C<0>>;
    Ctx ctx{{Vector{Mod{1}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 2) == Vector{Mod{1}, Mod{0}});
  }

  SECTION("shift") {
    using Ctx = PolyCtxT<Mod, 1, Shift<C<0>, 1>>;
    Ctx ctx{{Vector{Mod{1}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 3) == Vector{Mod{0}, Mod{1}, Mod{0}});
  }

  SECTION("neg") {
    using Ctx = PolyCtxT<Mod, 1, Neg<C<0>>>;
    Ctx ctx{{Vector{Mod{1}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 2) == Vector{-Mod{1}, Mod{0}});
  }

  SECTION("integral") {
    // \int (1 + z) = z + 1/2 z^2
    using Ctx = PolyCtxT<Mod, 1, Integral<C<0>>>;
    Ctx ctx{{Vector{Mod{1}, Mod{1}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 3) == Vector{Mod{0}, Mod{1}, Mod{2}.inv()});
  }

  SECTION("add") {
    using Ctx = PolyCtxT<Mod, 2, Add<C<0>, C<1>>>;
    Ctx ctx{{Vector{Mod{1}}, Vector{Mod{0}, Mod{2}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 3) == Vector{Mod{1}, Mod{2}, Mod{0}});
  }

  SECTION("sub") {
    using Ctx = PolyCtxT<Mod, 2, Sub<C<0>, C<1>>>;
    Ctx ctx{{Vector{Mod{1}}, Vector{Mod{0}, Mod{2}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 3) == Vector{Mod{1}, -Mod{2}, Mod{0}});
  }

  SECTION("custom_op") {
    // partial sum
    using Ctx = PolyCtxT<Mod, 1, CustomOp<C<0>>>;
    Ctx ctx{{Vector{Mod{1}, Mod{2}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 3) == Vector{Mod{1}, Mod{3}, Mod{3}});
  }

  // f(z) = f(z) * (z + z^2) + 1
  const Vector FIBS_5{Mod{1}, Mod{1}, Mod{2}, Mod{3}, Mod{5}};
  Mod FIB_100000{56136314};

  SECTION("fib_lazy") {
    using Ctx = PolyCtxT<Mod, 2, Add<LazyMul<Var<0>, C<0>>, C<1>>>;
    Ctx ctx{{Vector{Mod{0}, Mod{1}, Mod{1}}, {Mod{1}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 5) == FIBS_5);
    REQUIRE(f[100000] == FIB_100000);
  }

  SECTION("fib_semi") {
    using Ctx = PolyCtxT<Mod, 2, Add<MulSemi<Var<0>, C<0>>, C<1>>>;
    Ctx ctx{{Vector{Mod{0}, Mod{1}, Mod{1}}, {Mod{1}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 5) == FIBS_5);
    REQUIRE(f[100000] == FIB_100000);
  }

  SECTION("semi_bench") {
    constexpr int n = 100'000;
    Vector c0(n + 1);
    for (int i = 0; i <= n; ++i) {
      c0[i] = Mod{i};
    }

    BENCHMARK("opt1") {
      using Ctx = PolyCtxT<Mod, 2, Add<MulSemi<Var<0>, C<0>>, C<1>>>;
      Ctx ctx{{c0, {Mod{1}}}};
      auto &f = ctx.var_store<0>();
      REQUIRE(f[n] == Mod{189040980});
      return f[n];
    };
  }

  SECTION("fib_three_vars") {
    // g(z) = f(z) * z
    // h(z) = f(z) * z^2
    // f(z) = g(z) + h(z) + 1
    using Ctx = PolyCtxT<Mod, 1, Cache<Add<Add<Var<1>, Var<2>>, C<0>>>,
                         Shift<Var<0>, 1>, Shift<Var<0>, 2>>;
    Ctx ctx{{Vector{Mod{1}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 5) == FIBS_5);
    REQUIRE(f[100000] == FIB_100000);
  }

  SECTION("catalan") {
    // f(z) = f(z) * f(z) * z + 1
    using Ctx = PolyCtxT<Mod, 1, Add<Shift<MulFull<Var<0>, Var<0>>, 1>, C<0>>>;
    Ctx ctx{{Vector{Mod{1}}}};
    auto &f = ctx.var_store<0>();
    REQUIRE(take(f, 10) == std::vector<Mod>{Mod{1}, Mod{1}, Mod{2}, Mod{5},
                                            Mod{14}, Mod{42}, Mod{132},
                                            Mod{429}, Mod{1430}, Mod{4862}});
    REQUIRE(f[100000] == Mod{944488806});
    // NOTE: verified
    // sage: catalan_number(100000).mod(998244353)
    // 944488806
  }
}
