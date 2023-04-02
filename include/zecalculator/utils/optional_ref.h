#pragma once

#include <optional>
#include <memory>

namespace zc {

template <class T>
using optional_ref = std::optional<std::reference_wrapper<T>>;

}
