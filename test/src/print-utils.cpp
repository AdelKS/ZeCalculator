#include <boost/ut.hpp>
#include <zecalculator/test-utils/magic_enum.h>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/parsing/data_structures/tree.h>
#include <zecalculator/utils/utils.h>

namespace zc {

namespace parsing {
namespace tokens {

std::ostream& operator<<(std::ostream& os, const tokens::Text& txt_token)
{
  os << txt_token.name << " at (" << txt_token.substr_info.begin  << ", " << txt_token.substr_info.size << ") ";
  return os;
}

}

std::ostream &operator<<(std::ostream &os, const Token &token)
{
  os << static_cast<const TokenType&>(token);
  return os;
}

std::ostream& operator << (std::ostream& os, const Error& err)
{
  os << magic_enum::enum_name(err.error_type)
     << " at "
     << err.token;

  return os;
}

}

namespace ast {

void syntax_node_print_helper(std::ostream& os, const Tree& node, size_t padding = 0)
{
  const std::string padding_str(padding, ' ');

  std::visit(overloaded{[&](std::monostate) {
                          os << padding_str << "empty tree " << std::endl;
                        },
                        [&](const node::Function &f) {
                          os << padding_str << "Function " << f << " {"
                             << std::endl;
                          for (const Tree &subnode : f.subnodes)
                            syntax_node_print_helper(os, subnode, padding + 2);
                        },
                        [&](const node::Variable &v) {
                          os << padding_str << "Variable " << v
                             << std::endl;
                        },
                        [&](const node::Number &n) {
                          os << padding_str << "Number " << n
                             << std::endl;
                        }},
             node);
}

std::ostream &operator<<(std::ostream &os, const Tree &node) {
  os << std::endl;
  syntax_node_print_helper(os, node);
  return os;
}

}

namespace eval {

std::ostream& operator << (std::ostream& os, const Error& err)
{
  os << magic_enum::enum_name(err.error_type)
     << " at "
     << err.token;

  return os;
}

}

}
