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

void syntax_node_print_helper(std::ostream& os, const SyntaxTree& node, size_t padding = 0)
{
  const std::string padding_str(padding, ' ');
  os << padding_str << magic_enum::enum_name(node.type) << ' ' << node.str << ' ';
  if (not node.subnodes.empty())
  {
    os << "{" << std::endl;

    for (const SyntaxTree& subnode: node.subnodes)
      syntax_node_print_helper(os, subnode, padding+2);

    os << padding_str << "}";
  }
  os << std::endl;
}

std::ostream& operator << (std::ostream& os, const SyntaxTree& node)
{
  os << std::endl;
  syntax_node_print_helper(os, node);
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
