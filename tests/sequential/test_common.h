#pragma once

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#define EXPECT_TRUE(x)                                                         \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::cerr << "EXPECT_TRUE failed: " #x << " at " << __FILE__ << ":"      \
                << __LINE__ << "\n";                                          \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

#define EXPECT_EQ(a, b)                                                        \
  do {                                                                         \
    const auto va = (a);                                                       \
    const auto vb = (b);                                                       \
    if (!(va == vb)) {                                                         \
      std::cerr << "EXPECT_EQ failed: " #a "=" << va << " " #b "=" << vb      \
                << " at " << __FILE__ << ":" << __LINE__ << "\n";             \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

#define EXPECT_NE(a, b)                                                        \
  do {                                                                         \
    const auto va = (a);                                                       \
    const auto vb = (b);                                                       \
    if (va == vb) {                                                            \
      std::cerr << "EXPECT_NE failed: " #a "=" << va << " " #b "=" << vb      \
                << " at " << __FILE__ << ":" << __LINE__ << "\n";             \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

#define EXPECT_VEC_EQ(a, b)                                                    \
  do {                                                                         \
    const auto va = (a);                                                       \
    const auto vb = (b);                                                       \
    if (!(va == vb)) {                                                         \
      std::cerr << "EXPECT_VEC_EQ failed: " #a " != " #b " at " << __FILE__   \
                << ":" << __LINE__ << "\n";                                    \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

#define EXPECT_VEC_EQ_UNORDERED(a, b)                                          \
  do {                                                                         \
    auto va = (a);                                                             \
    auto vb = (b);                                                             \
    std::sort(va.begin(), va.end());                                           \
    std::sort(vb.begin(), vb.end());                                           \
    if (!(va == vb)) {                                                         \
      std::cerr << "EXPECT_VEC_EQ_UNORDERED failed: " #a " != " #b " at "      \
                << __FILE__ << ":" << __LINE__ << "\n";                       \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

#define EXPECT_NEAR(a, b, eps)                                                 \
  do {                                                                         \
    const double va = static_cast<double>(a);                                  \
    const double vb = static_cast<double>(b);                                  \
    const double veps = static_cast<double>(eps);                              \
    if (std::fabs(va - vb) > veps) {                                           \
      std::cerr << "EXPECT_NEAR failed: " #a "=" << va << " " #b "=" << vb    \
                << " eps=" << veps << " at " << __FILE__ << ":"               \
                << __LINE__ << "\n";                                           \
      std::exit(1);                                                            \
    }                                                                          \
  } while (0)

#define EXPECT_VEC_NEAR(a, b, eps)                                             \
  do {                                                                         \
    const auto va = (a);                                                       \
    const auto vb = (b);                                                       \
    if (va.size() != vb.size()) {                                              \
      std::cerr << "EXPECT_VEC_NEAR size mismatch at " << __FILE__ << ":"      \
                << __LINE__ << "\n";                                           \
      std::exit(1);                                                            \
    }                                                                          \
    for (std::size_t i = 0; i < va.size(); ++i) {                              \
      if (std::fabs(static_cast<double>(va[i]) - static_cast<double>(vb[i])) > \
          static_cast<double>(eps)) {                                          \
        std::cerr << "EXPECT_VEC_NEAR failed at i=" << i << " : " << va[i]     \
                  << " vs " << vb[i] << " at " << __FILE__ << ":"             \
                  << __LINE__ << "\n";                                         \
        std::exit(1);                                                          \
      }                                                                        \
    }                                                                          \
  } while (0)
