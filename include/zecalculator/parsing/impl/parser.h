#pragma once

/****************************************************************************
**  Copyright (c) 2023, Adel Kara Slimane <adel.ks@zegrapher.com>
**
**  This file is part of ZeCalculator.
**
**  ZeCalculators is free software: you may copy, redistribute and/or modify it
**  under the terms of the GNU Affero General Public License as published by the
**  Free Software Foundation, either version 3 of the License, or (at your
**  option) any later version.
**
**  This file is distributed in the hope that it will be useful, but
**  WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/


#include <zecalculator/error.h>
#include <zecalculator/math_objects/aliases.h>
#include <zecalculator/math_objects/impl/function.h>
#include <zecalculator/math_objects/impl/sequence.h>
#include <zecalculator/mathworld/impl/mathworld.h>
#include <zecalculator/parsing/data_structures/decl/ast.h>
#include <zecalculator/parsing/data_structures/impl/ast.h>
#include <zecalculator/parsing/data_structures/impl/fast.h>
#include <zecalculator/parsing/data_structures/impl/rpn.h>
#include <zecalculator/parsing/data_structures/impl/shared.h>
#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/parsing/decl/parser.h>

#include <cmath>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace zc {
namespace parsing {

inline std::optional<std::pair<double, size_t>> to_double(std::string_view view)
{
  std::optional<std::pair<double, size_t>> result = std::make_pair(0.0, 0);
  char* charAfter = const_cast<char*>(view.data()); // char that comes right after the number
  result->first = std::strtod(view.data(), &charAfter);

  if (charAfter == view.data() or result->first == HUGE_VAL)
    result.reset();
  else result->second = size_t(charAfter - view.data());

  return result;
}

inline tl::expected<std::vector<Token>, Error> tokenize(std::string_view expression)
{
  const std::string_view orig_expr = expression;
  std::vector<Token> parsing;

  auto is_seperator = [](const char ch) {

    static constexpr auto sep_table = []{
      static_assert(sizeof(char) == 1, "Assuming 1 byte char here");
      std::array<bool, 256> truth_table;
      truth_table.fill(false);
      for (const auto& op: tokens::operators)
        truth_table[uint8_t(op.token)] = true;
      for (char sep: std::array{' ', '(', ')'})
        truth_table[uint8_t(sep)] = true;
      return truth_table;
    }();

    return sep_table[uint8_t(ch)];
  };

  auto is_digit = [](unsigned char ch)
  {
    return std::isdigit(ch);
  };

  bool openingParenthesis = true, numberSign = true, value = true, canEnd = false,
       ope = false, closingParenthesis = false;

  enum : bool { FUNCTION_CALL_PTH, NORMAL_PTH};
  std::stack<bool> last_opened_pth;

  auto it = expression.cbegin();
  while (it != expression.cend())
  {
    const std::optional<char> next_char = (it+1 != expression.cend()) ? *(it+1) : std::optional<char>();

    // view on the single character pointed to by 'it'
    const std::string_view char_v = std::string_view(it, 1);

    if (is_digit(*it) or (next_char and numberSign and (*it == '-' or *it == '+') and is_digit(*next_char)))
    {
      auto double_val = to_double(std::string_view(it, expression.cend()));

      if (double_val)
      {
        const auto& [double_opt_val, processed_char_num] = *double_val;
        const std::string_view val_str_v(it, processed_char_num);
        // parsing successful
        parsing.emplace_back(double_opt_val, tokens::Text::from_views(val_str_v, orig_expr));
        it += processed_char_num;

        openingParenthesis = value = numberSign = false;
        ope = canEnd = closingParenthesis = true;
      }
      else
        return tl::unexpected(Error::wrong_format(
          Token(std::nan(""), tokens::Text::from_views(char_v, orig_expr)), std::string(expression)));
    }
    else if (auto opt_op = tokens::get_operator_description(*it))
    {
      auto&& char_txt = tokens::Text::from_views(char_v, orig_expr);
      if (ope)
      {
        parsing.emplace_back(opt_op->type, char_txt);

        openingParenthesis = value = true;
        ope = numberSign = closingParenthesis = canEnd = false;
        it++;
      }
      else return tl::unexpected(Error::unexpected(char_txt, std::string(expression)));
    }
    else if (*it == '(')
    {
      auto&& pth_txt = tokens::Text::from_views(char_v, orig_expr);
      if (openingParenthesis)
      {
        if (not parsing.empty() and parsing.back().type == tokens::FUNCTION)
        {
          parsing.emplace_back(tokens::FUNCTION_CALL_START, pth_txt);
          last_opened_pth.push(FUNCTION_CALL_PTH);
        }
        else
        {
          parsing.emplace_back(tokens::OPENING_PARENTHESIS, pth_txt);
          last_opened_pth.push(NORMAL_PTH);
        }

        numberSign = value = openingParenthesis = true;
        ope = closingParenthesis = canEnd = false;
        it++;
      }
      else return tl::unexpected(Error::unexpected(pth_txt, std::string(expression)));
    }
    else if (*it == ')')
    {
      auto&& pth_txt = tokens::Text::from_views(char_v, orig_expr);
      if (closingParenthesis and not last_opened_pth.empty())
      {
        if (last_opened_pth.top() == FUNCTION_CALL_PTH)
          parsing.emplace_back(tokens::FUNCTION_CALL_END, pth_txt);
        else parsing.emplace_back(tokens::CLOSING_PARENTHESIS, pth_txt);

        last_opened_pth.pop();

        ope = canEnd = closingParenthesis = true;
        value = numberSign = openingParenthesis = false;
        it++;
      }
      else return tl::unexpected(Error::unexpected(pth_txt, std::string(expression)));
    }
    else if (*it == ' ')
      // spaces are skipped
      it++;
    else
    {
      if (value)
      {
        // the only possibilities left are variables and functions
        // to differentiate between them, functions always are followed by an opening parenthesis
        // e.g. "cos("

        // skip anything that is not a separator
        // then keep what's in-between:
        // should be either a function name or variable name
        const auto token_begin = it;
        while (it != expression.cend() and
               not is_seperator(*it)) { it++; }

        auto&& token_txt = tokens::Text::from_views(std::string_view(token_begin, it), orig_expr);

        // skip spaces after the function name or variable name
        while (it != expression.cend() and
               *it == ' ') { it++; }

        if (it == expression.cend() or *it != '(')
        {
          // can only be a variable when we reach the end of the expression
          parsing.emplace_back(tokens::VARIABLE, token_txt);

          openingParenthesis = numberSign = value = false;
          canEnd = ope = closingParenthesis = true;
        }
        else
        {
          parsing.emplace_back(tokens::FUNCTION, token_txt);

          canEnd = closingParenthesis = ope = numberSign = value = false;
          openingParenthesis = true;
        }
      }
      else return tl::unexpected(Error::unexpected(tokens::Text::from_views(char_v, orig_expr), std::string(expression)));
    }
  }

  if (not last_opened_pth.empty())
  {
    auto&& expr_cend_txt = tokens::Text::from_views(std::string_view(it, 0), orig_expr);
    if (last_opened_pth.top() == FUNCTION_CALL_PTH)
      return tl::unexpected(Error::missing(expr_cend_txt, std::string(expression)));
    else return tl::unexpected(Error::missing(expr_cend_txt, std::string(expression)));
  }

  if (not canEnd)
  {
    auto&& expr_cend_txt = tokens::Text::from_views(std::string_view(it, 0), orig_expr);
    return tl::unexpected(Error::unexpected(expr_cend_txt, std::string(expression)));
  }

  return parsing;
}


inline bool is_valid_name(std::string_view name)
{
  auto parsing = tokenize(name);
  return parsing and parsing->size() == 1 and parsing->front().type == tokens::VARIABLE;
}

inline tl::expected<std::vector<std::span<const Token>::iterator>, Error>
  get_non_pth_enclosed_tokens(std::span<const Token> tokens, std::string_view expression)
{
  std::vector<std::span<const Token>::iterator> non_pth_enclosed_tokens;

  enum : bool { FUNCTION_CALL_PTH, NORMAL_PTH};
  std::stack<bool> last_opened_pth;

  // search for parentheses
  for (auto tokenIt = tokens.begin() ; tokenIt != tokens.end() ; tokenIt++)
  {
    if (tokenIt->type == tokens::OPENING_PARENTHESIS)
    {
      last_opened_pth.push(NORMAL_PTH);
    }
    else if (tokenIt->type == tokens::FUNCTION_CALL_START)
    {
      last_opened_pth.push(FUNCTION_CALL_PTH);
    }
    else if (tokenIt->type == tokens::FUNCTION_CALL_END)
    {
      if (not last_opened_pth.empty() and last_opened_pth.top() == FUNCTION_CALL_PTH)
        last_opened_pth.pop();
      else return tl::unexpected(Error::unexpected(*tokenIt, std::string(expression)));
    }
    else if (tokenIt->type == tokens::CLOSING_PARENTHESIS)
    {
      if (not last_opened_pth.empty() and last_opened_pth.top() == NORMAL_PTH)
        last_opened_pth.pop();
      else return tl::unexpected(Error::unexpected(*tokenIt, std::string(expression)));
    }
    // if not a parenthesis, and the token is not enclosed within parentheses, push it
    else if (last_opened_pth.empty())
      non_pth_enclosed_tokens.push_back(tokenIt);
  }

  return non_pth_enclosed_tokens;
}

/// @brief functor that maps a MathWorld::ConstDynMathObject to tl::expected<fast::fast, Error>
template <parsing::Type world_type>
struct VariableVisiter
{
  using Ret = tl::expected<parsing::FAST<world_type>, Error>;
  using T = parsing::FAST<world_type>;

  std::string expression;
  const tokens::Text& var_txt_token;

  Ret operator()(const GlobalConstant& global_constant)
  {
    return T{&global_constant};
  }
  Ret operator()(const Function<world_type>& f)
  {
    if (f.args_num() != 0) [[unlikely]]
      return tl::unexpected(Error::wrong_object_type(var_txt_token, expression));
    else return T{&f};
  }
  Ret operator()(auto&&)
  {
    return tl::unexpected(Error::wrong_object_type(var_txt_token, expression));
  }
};

template <parsing::Type world_type>
struct FunctionVisiter
{
  using Ret = tl::expected<FAST<world_type>, Error>;
  using T = FAST<world_type>;

  std::string expression;
  const AST& func;
  std::vector<FAST<world_type>> subnodes;

  template <size_t args_num>
  Ret operator()(const CppFunction<args_num>& f)
  {
    if (subnodes.size() != args_num) [[unlikely]]
      return tl::unexpected(Error::mismatched_fun_args(func.args_token(), expression));

    return T{&f, std::move(subnodes)};
  }
  Ret operator()(const zc::Function<world_type>& f)
  {
    if (subnodes.size() != f.args_num()) [[unlikely]]
      return tl::unexpected(Error::mismatched_fun_args(func.args_token(), expression));

    return T{&f, std::move(subnodes)};
  }
  Ret operator()(const zc::Sequence<world_type>& u)
  {
    if (subnodes.size() != 1) [[unlikely]]
      return tl::unexpected(Error::mismatched_fun_args(func.args_token(), expression));

    return T{&u, std::move(subnodes)};
  }
  Ret operator()(auto&&)
  {
    return tl::unexpected(Error::wrong_object_type(func.name, expression));
  }
};

template <Type type>
tl::expected<FAST<type>, Error> make_fast<type>::operator () (const AST& ast)
{
  using Ret = tl::expected<FAST<type>, Error>;
  return std::visit(
    utils::overloaded{
      [&](const AST::Func& func) -> Ret
      {
        std::vector<FAST<type>> operands;
        for (auto&& operand: func.subnodes)
        {
          auto expected_bound_node = (*this)(operand);
          if (expected_bound_node) [[likely]]
            operands.push_back(std::move(*expected_bound_node));
          else return tl::unexpected(expected_bound_node.error());
        }

        switch (func.type)
        {
          case AST::Func::OP_ASSIGN:
            return tl::unexpected(Error::not_implemented(ast.name, expression));

          case AST::Func::OP_ADD:
            assert(func.subnodes.size() == 2);
            return FAST<type>{.node = shared::node::Add{}, .subnodes = std::move(operands)};

          case AST::Func::OP_SUBTRACT:
            assert(func.subnodes.size() == 2);
            return FAST<type>{.node = shared::node::Subtract{}, .subnodes = std::move(operands)};

          case AST::Func::OP_MULTIPLY:
            assert(func.subnodes.size() == 2);
            return FAST<type>{.node = shared::node::Multiply{}, .subnodes = std::move(operands)};

          case AST::Func::OP_DIVIDE:
            assert(func.subnodes.size() == 2);
            return FAST<type>{.node = shared::node::Divide{}, .subnodes = std::move(operands)};

          case AST::Func::OP_POWER:
            assert(func.subnodes.size() == 2);
            return FAST<type>{.node = shared::node::Power{}, .subnodes = std::move(operands)};

          case AST::Func::FUNCTION:
          {
            auto* dyn_obj = math_world.get(ast.name.substr);
            if (not dyn_obj) [[unlikely]]
            {
              if (math_world.eq_object_inventory.contains(ast.name.substr))
                // if the object is referenced as an eq_object, that means it's simply in an invalid state
                return tl::unexpected(Error::object_in_invalid_state(ast.name, expression));

              // otherwise undefined
              else return tl::unexpected(Error::undefined_function(ast.name, expression));
            }
            if (not dyn_obj->has_value()) [[unlikely]]
              return tl::unexpected(Error::object_in_invalid_state(ast.name, expression));
            else return std::visit(FunctionVisiter<type>{expression, ast, std::move(operands)}, **dyn_obj);
          }
          case AST::Func::SEPARATOR:
            return tl::unexpected(Error::unexpected(ast.name, expression));

          default:
            [[unlikely]] throw std::runtime_error("Problem in ZeCalculator library");
        }
      },
      [&](const AST::InputVariable& input_var) -> Ret
      {
        return FAST<type>{shared::node::InputVariable{input_var.index}};
      },
      [&](const AST::Number& number) -> Ret
      {
        return FAST<type>{shared::node::Number{number.value}};
      },
      [&](AST::Variable) -> Ret
      {
        auto* dyn_obj = math_world.get(ast.name.substr);
        if (not dyn_obj) [[unlikely]]
          return tl::unexpected(Error::undefined_variable(ast.name, expression));
        if (not dyn_obj->has_value())
          return tl::unexpected(Error::object_in_invalid_state(ast.name, expression));
        else return std::visit(VariableVisiter<type>{expression, ast.name}, **dyn_obj);
      }
    },
    ast.dyn_data);
}

template <std::ranges::viewable_range Range>
  requires std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>
tl::expected<AST, Error> make_ast<Range>::operator () (std::span<const parsing::Token> tokens)
{
  using Ret = tl::expected<AST, Error>;

  if (tokens.empty()) [[unlikely]]
    return tl::unexpected(Error::empty_expression());

  auto get_current_sub_expr = [&](){
    const auto& end_token = tokens.back();
    size_t start = tokens.front().begin;
    size_t end = end_token.begin + end_token.substr.size();

    return tokens::Text{std::string(expression.substr(start, end - start)), start};
  };

  tokens::Text current_sub_expr = get_current_sub_expr();

  // when there's only a single token, it can only be number or a variable
  if (tokens.size() == 1)
  {
    const auto& back = tokens.back();
    switch(tokens.back().type)
    {
      case tokens::NUMBER:
        return AST::make_number(back, back.value);
        break;
      case tokens::VARIABLE:
      {
        auto it = std::ranges::find(input_vars, back.substr);
        if (it != input_vars.end())
        {
          // the index is computed with the distance between begin() and 'it'
          return AST::make_input_var(back, std::distance(input_vars.begin(), it));
        }
        else return AST::make_var(back);
        break;
      }
      default: [[unlikely]]
        // cannot be anything else
        return tl::unexpected(Error::unexpected(back, std::string(expression)));
        break;
    }
  }

  auto expected_non_pth_wrapped_tokens = get_non_pth_enclosed_tokens(tokens, expression);
  if (not expected_non_pth_wrapped_tokens.has_value())
    return tl::unexpected(expected_non_pth_wrapped_tokens.error());

  const auto& non_pth_enclosed_tokens = expected_non_pth_wrapped_tokens.value();

  // expression of the type "(...)"
  if (non_pth_enclosed_tokens.empty() and tokens.size() > 2 and
      tokens.front().type == tokens::OPENING_PARENTHESIS and
      tokens.back().type == tokens::CLOSING_PARENTHESIS)
  {
    return (*this)(std::span(tokens.begin()+1, tokens.end()-1));
  }

  // expression of the type "function(...)"
  else if (non_pth_enclosed_tokens.size() == 1 and tokens.size() > 3
           and tokens.front().type == tokens::FUNCTION
           and (tokens.begin() + 1)->type == tokens::FUNCTION_CALL_START
           and tokens.back().type == tokens::FUNCTION_CALL_END)
  {
    auto subnode = (*this)(std::span(tokens.begin()+2, tokens.end()-1));

    if (bool(subnode))
      return AST::make_func(AST::Func::FUNCTION,
                            tokens.front(),
                            current_sub_expr,
                            {std::move(*subnode)});
    else return tl::unexpected(std::move(subnode.error()));
  }

  // there are tokens that are not within parentheses
  else if (not non_pth_enclosed_tokens.empty())
  {
    // test, from right to left, within tokens that are not enclosed within parentheses
    // for operators in increasing order of priority. As soon as one is found, we take it
    std::optional<Ret> res;
    auto test_operator_priority = [&]<uint8_t priority>(std::integral_constant<uint8_t, priority>)
    {
      if (res)
        return;
      // we reverse the view, so the actual evaluation of the tree amounts of taking
      // left to right priority
      for (const auto& tok: non_pth_enclosed_tokens | std::views::reverse)
      {
        if (res)
          break;

        for (const tokens::Type op: tokens::get_operators<priority>())
        {
          if (res)
            break;

          if (tok->type == op)
          {
            // since we are dealing with infix binary operators, they can't be
            // at either the very beginning or the very end of the token list
            if (tok == tokens.begin() or tok + 1 == tokens.end()) [[unlikely]]
            {
              res = tl::unexpected(Error::unexpected(*tok, std::string(expression)));
              return;
            }

            std::vector<AST> subnodes;
            subnodes.reserve(2);

            auto left_hand_side = (*this)(std::span(tokens.begin(), tok));
            if (not left_hand_side.has_value()) [[unlikely]]
            {
              res = tl::unexpected(left_hand_side.error());
              return;
            }
            else if (op != tokens::Type::SEPARATOR and op != tokens::Type::OP_ASSIGN
                     and left_hand_side->is_func()
                     and left_hand_side->func_data().type == AST::Func::SEPARATOR) [[unlikely]]
            {
              // only separators and assignment can have separators as subnodes
              res = tl::unexpected(Error::unexpected(left_hand_side->name, std::string(expression)));
              return ;
            }
            else subnodes.push_back(std::move(*left_hand_side));

            auto right_hand_side = (*this)(std::span(tok + 1, tokens.end()));
            if (not right_hand_side.has_value()) [[unlikely]]
            {
              res = tl::unexpected(right_hand_side.error());
              return;
            }
            else if (op != tokens::Type::SEPARATOR and op != tokens::Type::OP_ASSIGN
                     and right_hand_side->is_func()
                     and right_hand_side->func_data().type == AST::Func::SEPARATOR) [[unlikely]]
            {
              // only separators can have separators as subnodes
              res = tl::unexpected(
                Error::unexpected(right_hand_side->name, std::string(expression)));
              return ;
            }
            else subnodes.push_back(std::move(right_hand_side.value()));

            res = AST::make_func(AST::Func::Type(op), *tok, current_sub_expr, std::move(subnodes));
          }
        }
      }
    };
    constexpr_for(test_operator_priority,
                  std::make_integer_sequence<uint8_t, tokens::max_priority + 1>());

    if (res)
      return std::move(*res);
  }

  return tl::unexpected(Error::unexpected(current_sub_expr, std::string(expression)));
}

template <std::ranges::viewable_range Range>
  requires std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>
AST mark_input_vars<Range>::operator () (const AST& tree)
{
  return std::visit(
    utils::overloaded{
      [&](const AST::Func& func) -> AST
      {
        std::vector<AST> subnodes;
        for (const AST& subnode: func.subnodes)
          subnodes.push_back((*this)(subnode));

        return AST::make_func(func.type,
                              tree.name,
                              func.full_expr,
                              std::move(subnodes));
      },
      [&](const AST::InputVariable&) -> AST
      {
        return tree;
      },
      [&](const AST::Number&) -> AST
      {
        return tree;
      },
      [&](AST::Variable) -> AST
      {
        auto it = std::ranges::find(input_vars, tree.name.substr);
        if (it != input_vars.end())
          return AST::make_input_var(tree.name, std::distance(input_vars.begin(), it));
        else return AST::make_var(tree.name);
      }
    },
    tree.dyn_data);
}

/// @brief transform nested two-argument separator nodes into a single separator nodes with many subnodes
inline AST flatten_separators(const AST& tree)
{
  return std::visit(
    utils::overloaded{
      [&](const AST::Func& func) -> AST
      {
        std::vector<AST> subnodes;
        for (const AST& subnode: func.subnodes)
          subnodes.push_back(flatten_separators(subnode));

        if (func.type == AST::Func::FUNCTION or func.type == AST::Func::SEPARATOR)
        {
          std::vector<AST> final_subnodes;
          final_subnodes.reserve(subnodes.size());

          for (AST& n: subnodes)
          {
            if (n.is_func() and n.func_data().type == AST::Func::SEPARATOR)
            {
              auto& nodes = n.func_data().subnodes;
              final_subnodes.reserve(final_subnodes.size() + nodes.size());
              std::ranges::move(nodes, std::back_inserter(final_subnodes));
            }
            else final_subnodes.push_back(std::move(n));
          }

          return AST::make_func(func.type, tree.name, func.full_expr, std::move(final_subnodes));
        }
        else
          return AST::make_func(func.type, tree.name, func.full_expr, std::move(subnodes));


      },
      [&](const AST::InputVariable&) -> AST
      {
        return tree;
      },
      [&](const AST::Number&) -> AST
      {
        return tree;
      },
      [&](AST::Variable) -> AST
      {
        return tree;
      }
    },
    tree.dyn_data);
}

namespace internal {
  inline void make_RPN(RPN& res, const FAST<zc::parsing::Type::RPN>& tree)
  {
    for (const auto& node: tree.subnodes)
      make_RPN(res, node);

    res.push_back(tree.node);
  }
}

inline RPN make_RPN(const FAST<Type::RPN>& tree)
{
  RPN res;
  internal::make_RPN(res, tree);
  return res;
}

/// @brief appends dependencies of 'ast' in 'deps'
struct direct_dependency_saver
{
  deps::Deps deps;

  direct_dependency_saver& operator () (const AST& ast)
  {
    std::visit(
      utils::overloaded{[&](const AST::Func& func)
                        {
                          // we don't register operators
                          if (func.type == AST::Func::FUNCTION)
                          {
                            deps::Dep& dep = deps[ast.name.substr];
                            dep.type = deps::Dep::FUNCTION;
                            dep.indexes.push_back(ast.name.begin);
                          }

                          std::ranges::for_each(func.subnodes, std::ref(*this));
                        },
                        [&](const AST::InputVariable&)
                        {
                        },
                        [&](const AST::Number&)
                        {
                        },
                        [&](AST::Variable)
                        {
                          deps::Dep& dep = deps[ast.name.substr];
                          dep.type = deps::Dep::VARIABLE;
                          dep.indexes.push_back(ast.name.begin);
                        }},
      ast.dyn_data);
    return *this;
  }
};

inline deps::Deps direct_dependencies(const AST& ast)
{
  return std::move(direct_dependency_saver{}(ast).deps);
}

} // namespace parsing
} // namespace zc
