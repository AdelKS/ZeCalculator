#include <stdexcept>
#include <sstream>

#include <zecalculator/utils/token.h>

namespace zc {

Token::Token(
  Type type,
  std::string_view str_v,
  std::optional<std::variant<Operator, double>> type_value)
  : type(type), str_v(str_v)
{
  switch(type)
  {
  case Type::NUMBER:
    value = std::get<double>(type_value.value());
    break;
  case Type::OPERATOR:
    op = std::get<Operator>(type_value.value());
    break;
  default: ;
  }
}

}
