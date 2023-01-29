#pragma once

#include "mod_wrapper.h"

#include <cstdint>
#include <limits>

namespace mod {

template <typename M_, int PHANTOM> struct DynModBaseT {
  using M = M_;

  void set_mod(M mod_) { mod = mod_; }

  M get_mod() const { return mod; }

private:
  using M2 = m2_t<M>;

public:
  M reduce(M2 x) { return x % mod; }

private:
  M mod;
};

template <int PHANTOM = 0>
using DynMod64T = ModWrapperT<DynModBaseT<uint64_t, PHANTOM>>;
template <int PHANTOM = 0>
using DynModT = ModWrapperT<DynModBaseT<uint32_t, PHANTOM>>;

} // namespace mod

using mod::DynMod64T;
using mod::DynModT;
