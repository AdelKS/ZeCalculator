#include <zecalculator/test-utils/print-utils.h>

namespace zc {

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