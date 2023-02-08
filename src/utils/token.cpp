#include <stdexcept>
#include <sstream>

#include <zecalculator/utils/token.h>

namespace zc {

Token::Operator::Operator(const char op_char)
{
  switch(op_char)
  {
  case '+':
    op = PLUS;
    break;
  case '-':
    op = MINUS;
    break;
  case '*':
    op = MULTIPLY;
    break;
  case '/':
    op = DIVIDE;
    break;
  case '^':
    op = POWER;
    break;
  [[unlikely]] default:
    throw std::invalid_argument("not an operator");
  }
}


char Token::Operator::name() const
{
  switch(op)
  {
  case PLUS:
    return '+';
  case MINUS:
    return '-';
  case MULTIPLY:
    return '*';
  case DIVIDE:
    return '/';
  case POWER:
    return '^';
  [[unlikely]] default:
    throw std::invalid_argument("not an operator");
  }
}

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

std::ostream& operator << (std::stringstream& os, const Token& token)
{
  switch(token.type)
  {
  case Token::Type::NUMBER:
    os << std::to_string(token.value.value());
    break;
  case Token::Type::VARIABLE:
    os << "var=" << token.str_v;
    break;
  case Token::Type::FUNCTION:
    os << "func=" << token.str_v;
    break;
  case Token::Type::OPERATOR:
    os << token.op.value().name();
    break;
  default:
    os << token.type_name();
  }

  return os;
}

}
