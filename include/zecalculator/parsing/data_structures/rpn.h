#pragma once

#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/parsing/data_structures/tree.h>
#include <vector>

namespace zc {
namespace rpn {

/// @brief represents a mathematical expression in reverse polish / postfix notation
using Token = std::variant<std::monostate,
                           parsing::tokens::Function,
                           parsing::tokens::Variable,
                           parsing::tokens::Number>;
using RPN = std::vector<Token>;

}
}
