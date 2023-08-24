#include <boost/ut.hpp>
#include <zecalculator/test-utils/magic_enum.h>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/utils/syntax_tree.h>
#include <zecalculator/utils/utils.h>

namespace zc {

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

void syntax_node_print_helper(std::ostream& os, const SyntaxTree& node, size_t padding = 0)
{
  const std::string padding_str(padding, ' ');

  std::visit(overloaded{[&](std::monostate) {
                          os << padding_str << "empty tree " << std::endl;
                        },
                        [&](const FunctionNode &f) {
                          os << padding_str << "Function " << f << " {"
                             << std::endl;
                          for (const SyntaxTree &subnode : f.subnodes)
                            syntax_node_print_helper(os, subnode, padding + 2);
                        },
                        [&](const VariableNode &v) {
                          os << padding_str << "Variable " << v
                             << std::endl;
                        },
                        [&](const NumberNode &n) {
                          os << padding_str << "Number " << n
                             << std::endl;
                        }},
             node);
}

std::ostream &operator<<(std::ostream &os, const SyntaxTree &node) {
  os << std::endl;
  syntax_node_print_helper(os, node);
  return os;
}

std::ostream& operator << (std::ostream& os, const ParsingError& err)
{
  os << magic_enum::enum_name(err.error_type)
     << " at "
     << err.token;

  return os;
}

std::ostream& operator << (std::ostream& os, const EvaluationError& err)
{
  os << magic_enum::enum_name(err.error_type)
     << " at "
     << err.node;

  return os;
}

}
