#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/test-utils/magic_enum.h>

namespace zc {

std::ostream& operator << (std::ostream& os, const Token& token)
{
  os << magic_enum::enum_name(token.type)
     << " "
     << token.str_v;

  return os;
}

std::ostream& operator << (std::ostream& os, const Error& err)
{
  os << magic_enum::enum_name(err.error_type)
     << " "
     << magic_enum::enum_name(err.token_type)
     << " at "
     << err.where;

  return os;
}

}
