#pragma once

#include <variant>

#include <zecalculator/utils/utils.h>

namespace zc {
namespace utils {

template <class T>
struct variant_convert;

template <class... U>
struct variant_convert<std::variant<U...>>
{
  template <class... T>
  std::variant<U...> operator () (const std::variant<T...>& var)
  {
    return std::visit(
      overloaded{
        [&](const auto& v) -> std::variant<U...>
        {
          return v;
        }
      },
      var
    );
  }
};


}
}
