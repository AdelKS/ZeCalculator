#pragma once

#include <functional>

namespace zc {

template <class T>
using cref = std::reference_wrapper<const T>;

template <class T>
using ref = std::reference_wrapper<T>;

template <class T>
using ptr = T*;

template <class T>
using cst_ptr = const T*;

}
