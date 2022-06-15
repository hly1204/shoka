#include "ntt_util.h"

#include <cstdlib>
#include <memory>
#include <vector>

namespace ntt {

/*
 * Operation in Finite Polynomial Ring
 * e.g. 1 / f(z) is not finite and should be computed as 1 / f(z) mod z^n
 */
template <typename NTT>
struct FinitePolyFactoryT
    : std::enable_shared_from_this<FinitePolyFactoryT<NTT>> {
private:
  using Mod = typename NTT::Mod;
  using Factory = FinitePolyFactoryT<NTT>;

public:
  using Ptr = std::shared_ptr<Factory>;

  static Ptr create(int max_deg) { return Ptr(new Factory(max_deg)); }

  struct Poly : public std::vector<Mod> {
    Poly() : std::vector<Mod>(), factory(LocalSharedPtr<Factory>(nullptr)) {}

    int deg() const { return static_cast<int>(std::vector<Mod>::size()) - 1; }

    Poly operator+(const Poly &o) const {
      if (deg() < o.deg()) {
        return o + *this;
      }
      Poly r = factory->make(*this);
      for (int i = 0; i <= o.deg(); ++i) {
        r[i] += o[i];
      }
      return r;
    }

    Poly &operator+=(Poly &o) { return *this = *this += o; }

    Poly operator-(const Poly &o) const {
      int max_deg = std::max(deg(), o.deg());
      Poly r = factory->make(max_deg + 1);
      int min_deg = std::min(deg(), o.deg());
      for (int i = 0; i <= min_deg; ++i) {
        r[i] = (*this)[i] - o[i];
      }
      for (int i = min_deg + 1; i <= deg(); ++i) {
        r[i] = (*this)[i];
      }
      for (int i = min_deg + 1; i <= o.deg(); ++i) {
        r[i] = -o[i];
      }
      return r;
    }

    Poly &operator-=(Poly &o) { return *this = *this -= o; }

    Poly operator*(const Poly &o) const {
      int deg_plus_1 = deg() + o.deg() + 1;
      if (deg_plus_1 <= 16) {
        std::vector<Mod> result(deg_plus_1);
        for (int i = 0; i <= deg(); ++i) {
          for (int j = 0; j <= o.deg(); ++j) {
            result[i + j] += (*this)[i] * o[j];
          }
        }
        return factory->make(std::move(result));
      }

      Mod *b0 = factory->template raw_buffer<0>();
      Mod *b1 = factory->template raw_buffer<1>();

      int n = min_power_of_two(deg_plus_1);
      factory->assert_max_n(n);
      copy_and_fill0(n, b0, *this);
      NTT::dif(n, b0);
      copy_and_fill0(n, b1, o);
      NTT::dif(n, b1);
      dot_product_and_dit(n, Mod(n).inverse(), b0, b0, b1);
      return factory->make(std::vector<Mod>(b0, b0 + deg_plus_1));
    }

    Poly &operator*=(const Poly &o) { return *this = *this * o; }

  private:
    friend struct FinitePolyFactoryT;

    template <typename... Args>
    Poly(Ptr &&factory_, Args &&...args)
        : std::vector<Mod>(std::forward<Args>(args)...),
          factory(std::move(factory_)) {}

    Ptr factory;
  };

  template <typename... Args> Poly make(Args &&...args) {
    Poly p{std::enable_shared_from_this<Factory>::shared_from_this(),
           std::forward<Args>(args)...};
    return p;
  }

private:
  friend struct Poly;

  FinitePolyFactoryT(int max_deg)
      : max_n{min_power_of_two(max_deg + 1)}, buffer{std::vector<Mod>(max_n),
                                                     std::vector<Mod>(max_n)} {}

  static void copy_and_fill0(int n, Mod *dst, int m, const Mod *src) {
    m = std::min(n, m);
    std::copy(src, src + m, dst);
    std::fill(dst + m, dst + n, Mod(0));
  }

  static void copy_and_fill0(int n, Mod *dst, const std::vector<Mod> &src) {
    copy_and_fill0(n, dst, src.size(), src.data());
  }

  static void dot_product_and_dit(int n, Mod inv_n, Mod *out, const Mod *a,
                                  const Mod *b) {
    for (int i = 0; i < n; ++i) {
      out[i] = inv_n * a[i] * b[i];
    }
    NTT::dit(n, out);
  }

  void assert_max_n(int n) const {
    if (n > max_n) {
      throw std::invalid_argument("insufficient buffer");
    }
  }

  template <int index> Mod *raw_buffer() { return buffer[index].data(); }

  int max_n;
  std::array<std::vector<Mod>, 2> buffer;
};

} // namespace ntt
