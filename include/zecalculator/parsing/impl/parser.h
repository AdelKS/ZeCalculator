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
#include <zecalculator/parsing/data_structures/impl/ast.h>
#include <zecalculator/parsing/data_structures/impl/rpn.h>
#include <zecalculator/parsing/data_structures/impl/uast.h>
#include <zecalculator/parsing/decl/parser.h>

#include <cmath>
#include <numeric>
#include <optional>
#include <ranges>
#include <string_view>

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
    static constexpr std::array separators = {'+', '-', '*', '/', '^', ' ', '(', ')'};
    return std::ranges::any_of(separators, [&ch](const char op){ return op == ch; });
  };

  auto is_digit = [](unsigned char ch)
  {
    return std::isdigit(ch);
  };

  auto is_argument_separator = [](const char ch)
  {
    return ch == ',' or ch == ';';
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
        parsing.push_back(tokens::Number(double_opt_val, tokens::Text(val_str_v, orig_expr)));
        it += processed_char_num;

        openingParenthesis = value = numberSign = false;
        ope = canEnd = closingParenthesis = true;
      }
      else
        return tl::unexpected(Error::wrong_format(
          tokens::Number(std::nan(""), tokens::Text(char_v, orig_expr)), std::string(expression)));
    }
    else if (tokens::is_operator(*it))
    {
      if (ope)
      {
        auto push_op = [&]<char op>(std::integral_constant<char, op>)
        {
          if (*it == op)
            parsing.push_back(tokens::Operator<op, 2>(char_v, orig_expr));
        };
        utils::for_int_seq(push_op, tokens::OperatorSequence());

        openingParenthesis = value = true;
        ope = numberSign = closingParenthesis = canEnd = false;
        it++;
      }
      else return tl::unexpected(Error::unexpected(tokens::Text(char_v, orig_expr), std::string(expression)));
    }
    else if (*it == '(')
    {
      if (openingParenthesis)
      {
        if (not parsing.empty() and std::holds_alternative<tokens::Function>(parsing.back()))
        {
          parsing.emplace_back(tokens::FunctionCallStart(char_v, orig_expr));
          last_opened_pth.push(FUNCTION_CALL_PTH);
        }
        else
        {
          parsing.emplace_back(tokens::OpeningParenthesis(char_v, orig_expr));
          last_opened_pth.push(NORMAL_PTH);
        }

        numberSign = value = openingParenthesis = true;
        ope = closingParenthesis = canEnd = false;
        it++;
      }
      else return tl::unexpected(Error::unexpected(tokens::OpeningParenthesis(char_v, orig_expr), std::string(expression)));
    }
    else if (*it == ')')
    {
      if (closingParenthesis and not last_opened_pth.empty())
      {
        if (last_opened_pth.top() == FUNCTION_CALL_PTH)
        {
          parsing.emplace_back(tokens::FunctionCallEnd(char_v, orig_expr));
        }
        else parsing.emplace_back(tokens::ClosingParenthesis(char_v, orig_expr));

        last_opened_pth.pop();

        ope = canEnd = closingParenthesis = true;
        value = numberSign = openingParenthesis = false;
        it++;
      }
      else return tl::unexpected(Error::unexpected(tokens::ClosingParenthesis(char_v, orig_expr), std::string(expression)));
    }
    else if (*it == ' ')
      // spaces are skipped
      it++;
    else if (is_argument_separator(*it))
    {
      if (not last_opened_pth.empty() and last_opened_pth.top() == FUNCTION_CALL_PTH)
        parsing.emplace_back(tokens::FunctionArgumentSeparator(char_v, orig_expr));
      else return tl::unexpected(Error::unexpected(tokens::FunctionArgumentSeparator(char_v, orig_expr), std::string(expression)));

      openingParenthesis = numberSign = value = true;
      canEnd = ope = closingParenthesis = false;
      it++;
    }
    else
    {
      if (value)
      {
        // the only possibilities left are variables and functions
        // to differentiate between them, functions always are followed by an opening parenthesis
        // e.g. "cos("

        const auto token_begin = it;
        while (it != expression.cend() and
               not is_seperator(*it) and
               not is_argument_separator(*it)) { it++; }

        const std::string_view token_v(token_begin, it);

        if (it == expression.cend() or *it != '(')
        {
          // can only be a variable when we reach the end of the expression
          parsing.emplace_back(tokens::Variable(token_v, orig_expr));

          openingParenthesis = numberSign = value = false;
          canEnd = ope = closingParenthesis = true;
        }
        else
        {
          parsing.emplace_back(tokens::Function(token_v, orig_expr));

          canEnd = closingParenthesis = ope = numberSign = value = false;
          openingParenthesis = true;
        }
      }
      else return tl::unexpected(Error::unexpected(tokens::Unkown(char_v, orig_expr), std::string(expression)));
    }
  }

  if (not last_opened_pth.empty())
  {
    std::string_view expr_cend = std::string_view(it, 0);
    if (last_opened_pth.top() == FUNCTION_CALL_PTH)
      return tl::unexpected(Error::missing(tokens::FunctionCallEnd(expr_cend, orig_expr), std::string(expression)));
    else return tl::unexpected(Error::missing(tokens::ClosingParenthesis(expr_cend, orig_expr), std::string(expression)));
  }

  if (not canEnd)
  {
    std::string_view expr_cend = std::string_view(it, 0);
    return tl::unexpected(Error::unexpected(tokens::EndOfExpression(expr_cend, orig_expr), std::string(expression)));
  }

  return parsing;
}


inline bool is_valid_name(std::string_view name)
{
  auto parsing = tokenize(name);
  return parsing and parsing->size() == 1
         and std::holds_alternative<tokens::Variable>(parsing->front());
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
    if (std::holds_alternative<tokens::OpeningParenthesis>(*tokenIt))
    {
      last_opened_pth.push(NORMAL_PTH);
    }
    else if (std::holds_alternative<tokens::FunctionCallStart>(*tokenIt))
    {
      last_opened_pth.push(FUNCTION_CALL_PTH);
    }
    else if (std::holds_alternative<tokens::FunctionCallEnd>(*tokenIt))
    {
      if (not last_opened_pth.empty() and last_opened_pth.top() == FUNCTION_CALL_PTH)
        last_opened_pth.pop();
      else return tl::unexpected(Error::unexpected(text_token(*tokenIt), std::string(expression)));
    }
    else if (std::holds_alternative<tokens::ClosingParenthesis>(*tokenIt))
    {
      if (not last_opened_pth.empty() and last_opened_pth.top() == NORMAL_PTH)
        last_opened_pth.pop();
      else return tl::unexpected(Error::unexpected(text_token(*tokenIt), std::string(expression)));
    }
    // if not a parenthesis, and the token is not enclosed within parentheses, push it
    else if (last_opened_pth.empty())
      non_pth_enclosed_tokens.push_back(tokenIt);
  }

  return non_pth_enclosed_tokens;
}

/// @brief functor that maps a MathWorld::ConstDynMathObject to tl::expected<ast::AST, Error>
template <parsing::Type world_type>
struct VariableVisiter
{
  using Ret = tl::expected<parsing::AST<world_type>, Error>;
  std::string expression;
  const tokens::Text& var_txt_token;

  Ret operator()(const GlobalConstant<world_type>& global_constant)
  {
    return std::make_unique<ast::node::Node<world_type>>(
      shared::node::GlobalConstant<world_type>(var_txt_token, &global_constant));
  }
  Ret operator()(const zc::GlobalVariable<world_type>& global_variable)
  {
    return std::make_unique<ast::node::Node<world_type>>(
      ast::node::GlobalVariable<world_type>(var_txt_token, &global_variable));
  }
  Ret operator()(auto&&)
  {
    return tl::unexpected(Error::wrong_object_type(var_txt_token, expression));
  }
};

template <parsing::Type world_type>
struct FunctionVisiter
{
  using Ret = tl::expected<AST<world_type>, Error>;
  std::string expression;
  const tokens::Text& func_txt_token;
  std::vector<AST<world_type>> subnodes;

  /// @brief moves the elements of 'subnodes' into an array
  template <size_t args_num>
  std::array<AST<world_type>, args_num> get_subnode_arr()
  {
    assert(subnodes.size() == args_num);

    auto helper = [&]<size_t...i>(std::index_sequence<i...>)
    {
      return std::array<AST<world_type>, args_num>{std::move(subnodes[i])...};
    };
    return helper(std::make_index_sequence<args_num>());
  }


  template <size_t args_num>
  Ret operator()(const CppFunction<world_type, args_num>& f)
  {
    if (subnodes.size() != args_num) [[unlikely]]
      return tl::unexpected(Error::mismatched_fun_args(func_txt_token, expression));

    return std::make_unique<ast::node::Node<world_type>>(
      ast::node::CppFunction<world_type, args_num>(
        func_txt_token, &f, get_subnode_arr<args_num>()));
  }
  template <size_t args_num>
  Ret operator()(const zc::Function<world_type, args_num>& f)
  {
    if (subnodes.size() != args_num) [[unlikely]]
      return tl::unexpected(Error::mismatched_fun_args(func_txt_token, expression));

    return std::make_unique<ast::node::Node<world_type>>(
      ast::node::Function<world_type, args_num>(func_txt_token, &f, get_subnode_arr<args_num>()));
  }
  Ret operator()(const zc::Sequence<world_type>& u)
  {
    if (subnodes.size() != 1) [[unlikely]]
      return tl::unexpected(Error::mismatched_fun_args(func_txt_token, expression));

    return std::make_unique<ast::node::Node<world_type>>(
      ast::node::Sequence<world_type>(func_txt_token, &u, std::move(subnodes.front())));
  }
  Ret operator()(auto&&)
  {
    return tl::unexpected(Error::wrong_object_type(func_txt_token, expression));
  }
};

template <Type type>
struct bind
{
  std::string expression;
  const MathWorld<type>& math_world;

  tl::expected<AST<type>, Error> operator () (const UAST& uast)
  {
    using Ret = tl::expected<AST<type>, Error>;

    return std::visit(
      utils::overloaded{
        [](const shared::node::InputVariable& in_var) -> Ret
        {
          return in_var;
        },
        [](const shared::node::Number& num) -> Ret
        {
          return num;
        },
        [&](const uast::node::Function& func) -> Ret
        {
          std::vector<AST<type>> operands;
          for (auto&& operand: func.subnodes)
          {
            auto expected_bound_node = (*this)(operand);
            if (expected_bound_node) [[likely]]
              operands.push_back(std::move(*expected_bound_node));
            else return tl::unexpected(expected_bound_node.error());
          }

          auto* dyn_obj = math_world.get(func.name);
          if (not dyn_obj) [[unlikely]]
            return tl::unexpected(Error::undefined_function(func, expression));
          else return std::visit(FunctionVisiter<type>{expression, func, std::move(operands)}, dyn_obj->variant);
        },
        [&](const uast::node::Variable& var) -> Ret
        {
          auto* dyn_obj = math_world.get(var.name);
          if (not dyn_obj) [[unlikely]]
            return tl::unexpected(Error::undefined_variable(var, expression));
          else return std::visit(VariableVisiter<type>{expression, var}, dyn_obj->variant);
        },
        [&]<char op, size_t args_num>(const uast::node::Operator<op, args_num>& ope) -> Ret
        {
          auto get_operator = [&]<size_t... i>(std::index_sequence<i...>) -> Ret
          {
            std::array<Ret, args_num> expected_operands = {(*this)(ope.operands[i])...};
            auto it = std::ranges::find_if_not(expected_operands, [](auto&& v){ return bool(v); });
            if (it != expected_operands.end())
              return tl::unexpected(it->error());
            else return ast::node::Operator<type, op, args_num>(ope, {std::move(*expected_operands[i])...});
          };
          return get_operator(std::make_index_sequence<args_num>());
        }
      },
      *uast
    );
  }
};

template <std::ranges::viewable_range Range>
  requires std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>
tl::expected<UAST, Error> make_uast(std::string_view expression, std::span<const parsing::Token> tokens, const Range& input_vars)
{
  using Ret = tl::expected<UAST, Error>;

  if (tokens.empty()) [[unlikely]]
    return tl::unexpected(Error::empty());

  // when there's only a single token, it can only be number or a variable
  if (tokens.size() == 1)
  {
    return std::visit(
      utils::overloaded{
        [&](const tokens::Number& num) -> Ret {
          return num;
        },
        [&](const tokens::Variable& var) -> Ret
        {
          // if variable is in 'input_vars' then treat it as such
          // this will avoid name lookup when we evaluate
          auto it = std::ranges::find(input_vars, var.name);
          if (it != input_vars.end())
            // the index is computed with the distance between begin() and 'it'
            return shared::node::InputVariable(var, std::distance(input_vars.begin(), it));
          else return var;
        },
        [&](const auto& anything_else) -> Ret {
          return tl::unexpected(Error::unexpected(anything_else, std::string(expression)));
        }},
      tokens.back());
  }

  auto expected_non_pth_wrapped_tokens = get_non_pth_enclosed_tokens(tokens, expression);
  if (not expected_non_pth_wrapped_tokens.has_value())
    return tl::unexpected(expected_non_pth_wrapped_tokens.error());

  const auto& non_pth_enclosed_tokens = expected_non_pth_wrapped_tokens.value();

  // expression of the type "(...)"
  if (non_pth_enclosed_tokens.empty() and tokens.size() > 2 and
      std::holds_alternative<tokens::OpeningParenthesis>(tokens.front()) and
      std::holds_alternative<tokens::ClosingParenthesis>(tokens.back()))
  {
    return make_uast(expression, std::span(tokens.begin()+1, tokens.end()-1), input_vars);
  }

  // expression of the type "function(...)"
  else if (non_pth_enclosed_tokens.size() == 1 and tokens.size() > 3 and
           std::holds_alternative<tokens::Function>(tokens.front()) and
           std::holds_alternative<tokens::FunctionCallStart>(*(tokens.begin()+1)) and
           std::holds_alternative<tokens::FunctionCallEnd>(tokens.back()))
  {
    /// @todo needs changing to support multi-argument functions
    /// @note here we expect functions that receive that receive a single argument
    auto non_pth_wrapped_args = get_non_pth_enclosed_tokens(
      std::span(tokens.begin() + 2, tokens.end() - 1), expression);

    if (not non_pth_wrapped_args.has_value())
      return tl::unexpected(non_pth_wrapped_args.error());

    // add the FunctionCallEnd token so we handle it in the loop
    non_pth_wrapped_args->push_back(tokens.end()-1);

    std::vector<UAST> subnodes;
    auto last_non_coma_token_it = tokens.begin()+2;
    for (auto tokenIt: *non_pth_wrapped_args)
    {
      if (std::holds_alternative<tokens::FunctionArgumentSeparator>(*tokenIt) or
          std::holds_alternative<tokens::FunctionCallEnd>(*tokenIt))
      {
        auto expected_func_argument = make_uast(expression, std::span(last_non_coma_token_it, tokenIt), input_vars);
        if (not expected_func_argument.has_value())
          return expected_func_argument;
        else subnodes.push_back(std::move(expected_func_argument.value()));
        last_non_coma_token_it = tokenIt+1;
      }
    }

    const auto func_txt_token = text_token(tokens.front());
    return uast::node::Function(func_txt_token, std::move(subnodes));

    // return ast::node::Function(text_token(tokens.front()), std::move(subnodes));
  }

  // there are tokens that are not within parentheses
  else if (not non_pth_enclosed_tokens.empty())
  {
    // we check for operations
    // loop through the expression by increasing operator priority
    // -> because the deepest parts of the syntax tree are to be calculated first
    auto get_node = [&]<char op>(std::integral_constant<char, op>,
                                 auto&& tokenIt) -> Ret
    {
      // we are not within parentheses, and we are at the right operator priority
      if (tokenIt == tokens.begin() or tokenIt + 1 == tokens.end())
        return tl::unexpected(Error::unexpected(text_token(*tokenIt), std::string(expression)));

      auto left_hand_side = make_uast(expression, std::span(tokens.begin(), tokenIt), input_vars);
      if (not left_hand_side.has_value())
        return left_hand_side;

      auto right_hand_side = make_uast(expression, std::span(tokenIt + 1, tokens.end()), input_vars);
      if (not right_hand_side.has_value())
        return right_hand_side;

      return uast::node::Operator<op, 2>(text_token(*tokenIt).substr_info.value().begin,
                                         std::array{std::move(left_hand_side.value()),
                                                    std::move(right_hand_side.value())});
    };

    // test, from right to left, within tokens that are not enclosed within parentheses
    // for operators in increasing order of priority. As soon as one is found, we take it
    std::optional<Ret> res;
    auto test_operator_list = [&]<size_t i>(std::integral_constant<size_t, i>)
    {
      // we reverse the view, so the actual evaluation of the tree amounts of taking
      // left to right priority
      for (const auto& tok: non_pth_enclosed_tokens | std::views::reverse)
      {
        auto test_op = [&]<size_t j>(std::integral_constant<size_t, j>)
        {
          constexpr char op = std::get<i>(tokens::operators)[j];
          if (not res and std::holds_alternative<tokens::Operator<op, 2>>(*tok))
            res = get_node(std::integral_constant<char, op>(), tok);
        };
        constexpr size_t ops_num = std::get<i>(tokens::operators).size();
        constexpr_for(test_op, std::make_index_sequence<ops_num>());
      }
    };
    constexpr_for(test_operator_list,
                  std::make_index_sequence<std::tuple_size_v<decltype(tokens::operators)>>());

    if (res)
      return std::move(*res);
  }

  // if we reach the end of this function, something is not right
  auto text_tokens = tokens | std::views::transform([](auto&& tok){ return text_token(tok); });
  tokens::Text unexpected_slice = std::accumulate(text_tokens.begin() + 1, text_tokens.end(), *text_tokens.begin());

  return tl::unexpected(Error::unexpected(unexpected_slice, std::string(expression)));
}

template <std::ranges::viewable_range Range>
  requires std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>
UAST mark_input_vars<Range>::operator () (const UAST& tree)
{
  return std::visit(utils::overloaded{
    [&](const shared::node::InputVariable& v) -> UAST { return v; },
    [&](const shared::node::Number& n) -> UAST { return n; },
    [&](const uast::node::Function& f) -> UAST {
      uast::node::Function f_copy(f, {});
      for (const UAST& sub_tree: f.subnodes)
        f_copy.subnodes.push_back((*this)(sub_tree));
      return f_copy;
    },
    [&](uast::node::Variable& v) -> UAST {
      auto it = std::ranges::find(input_vars, v.name);
      if (it != input_vars.end())
        return shared::node::InputVariable(v, std::distance(input_vars.begin(), it));
      else return v;
    },
    [&]<char op, size_t args_num>(const uast::node::Operator<op, args_num>& op_f) -> UAST {
      auto make_operands = [&]<size_t... i>(std::integer_sequence<size_t, i...>)
      {
        return std::array{(void(i), (*this)(op_f.operands[i]))...};
      };
      return uast::node::Operator<op, args_num>(op_f, make_operands(std::make_index_sequence<args_num>()));
    }},
    *tree);
}

struct RpnMaker
{
  RPN operator()(const ast::node::Sequence<Type::RPN>& seq)
  {
    RPN res = std::visit(*this, *seq.operand);

    res.push_back(rpn::node::Sequence(seq, seq.u));
    return res;
  }

  template <size_t args_num>
  RPN operator()(const ast::node::CppFunction<Type::RPN, args_num>& func)
  {
    RPN res;
    for (const AST<Type::RPN>& sub_node : func.operands)
    {
      RPN tmp = std::visit(*this, *sub_node);
      std::ranges::move(tmp, std::back_inserter(res));
    }

    res.push_back(rpn::node::CppFunction<args_num>(func, func.f));
    return res;
  }

  template <size_t args_num>
  RPN operator () (const ast::node::Function<Type::RPN, args_num>& func)
  {
    RPN res;
    for (const AST<Type::RPN>& sub_node : func.operands)
    {
      RPN tmp = std::visit(*this, *sub_node);
      std::ranges::move(tmp, std::back_inserter(res));
    }

    res.push_back(rpn::node::Function<args_num>(func, func.f));
    return res;
  }

  template <char op, size_t args_num>
  RPN operator () (const ast::node::Operator<Type::RPN, op, args_num>& func)
  {
    RPN res;
    for (const AST<Type::RPN>& sub_node : func.operands)
    {
      RPN tmp = std::visit(*this, *sub_node);
      std::ranges::move(tmp, std::back_inserter(res));
    }

    res.push_back(rpn::node::Operator<op, args_num>(func));
    return res;
  }

  RPN operator () (const shared::node::InputVariable& in_var)
  {
    return RPN(1, in_var);
  }

  RPN operator () (const shared::node::GlobalConstant<Type::RPN>& var)
  {
    return RPN(1, var);
  }

  RPN operator () (const shared::node::Number& number)
  {
    return RPN(1, number);
  }

};

inline RPN make_RPN(const AST<Type::RPN>& tree)
{
  return std::visit(RpnMaker{}, *tree);
}

template <std::ranges::viewable_range Range>
  requires std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>
deps::Deps direct_dependencies(const std::vector<parsing::Token>& tokens, const Range& input_vars)
{
  deps::Deps deps;

  for (const parsing::Token& tok: tokens)
    std::visit(
      utils::overloaded{
        [&](const parsing::tokens::Function& f) {
          deps.insert({f.name, deps::ObjectType::FUNCTION});
        },
        [&](const parsing::tokens::Variable& v) {
          if (std::ranges::count(input_vars, v.name) == 0)
            deps.insert({v.name, deps::ObjectType::VARIABLE});
        },
        [](auto&&){ /* no op */ },
      }, tok);

  return deps;
}


/// @brief appends dependencies of 'uast' in 'deps'
struct direct_dependency_saver
{
  deps::Deps deps;

  direct_dependency_saver& operator () (const UAST& uast)
  {
    std::visit(
      utils::overloaded{
        [&](const uast::node::Function& f)
        {
          deps.insert({f.name, deps::ObjectType::FUNCTION});
          std::ranges::for_each(f.subnodes, std::ref(*this));
          // if we do not use std::ref, a copy of this instance is taken
        },
        [&](const uast::node::Variable& v)
        {
          deps.insert({v.name, deps::ObjectType::VARIABLE});
        },
        [&]<char op, size_t args_num>(const uast::node::Operator<op, args_num>& ope)
        {
          std::ranges::for_each(ope.operands, std::ref(*this));
          // if we do not use std::ref, a copy of this instance is taken
        },
        [&](const shared::node::InputVariable&) { /* no op */ },
        [&](const shared::node::Number&) { /* no op */ },
      },
      *uast);

    return *this;
  }
};

deps::Deps direct_dependencies(const UAST& uast)
{
  return std::move(direct_dependency_saver{}(uast).deps);
}

} // namespace parsing
} // namespace zc
