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

#include <tuple>
#include <stack>
#include <numeric>

#include <zecalculator/utils/syntax_tree.h>
#include <zecalculator/utils/parser.h>
#include <zecalculator/utils/parsing_error.h>
#include <zecalculator/utils/utils.h>

namespace zc {

tl::expected<
  std::vector<std::span<Token>::iterator>,
  ParsingError
> get_non_pth_enclosed_tokens(std::span<Token> tokens)
{
  std::vector<std::span<Token>::iterator> non_pth_enclosed_tokens;

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
      else return tl::unexpected(ParsingError::unexpected(*tokenIt));
    }
    else if (std::holds_alternative<tokens::ClosingParenthesis>(*tokenIt))
    {
      if (not last_opened_pth.empty() and last_opened_pth.top() == NORMAL_PTH)
        last_opened_pth.pop();
      else return tl::unexpected(ParsingError::unexpected(*tokenIt));
    }
    // if not a parenthesis, and the token is not enclosed within parentheses, push it
    else if (last_opened_pth.empty())
      non_pth_enclosed_tokens.push_back(tokenIt);
  }

  return non_pth_enclosed_tokens;
}

tl::expected<SyntaxTree, ParsingError> make_tree(const std::span<Token> tokens)
{
  // when there's only a single token, it can only be number or a variable
  if (tokens.size() == 1)
  {
    const Token& single_token = tokens.back();
    tl::expected<SyntaxTree, ParsingError> ret;
    std::visit(
        overloaded{
            [&](const tokens::Number &num) {
              ret = NumberNode(num);
            },
            [&](const tokens::Variable &var) {
              ret = VariableNode{var};
            },
            [&](auto &&anything_else) {
              ret = tl::unexpected(ParsingError::unexpected(anything_else));
            }},
        single_token);
    return ret;
  }

  auto expected_non_pth_wrapped_tokens = get_non_pth_enclosed_tokens(tokens);
  if (not expected_non_pth_wrapped_tokens.has_value())
    return tl::unexpected(expected_non_pth_wrapped_tokens.error());

  const auto& non_pth_enclosed_tokens = expected_non_pth_wrapped_tokens.value();

  // experssion of the type "(...)"
  if (non_pth_enclosed_tokens.empty() and tokens.size() > 2 and
      std::holds_alternative<tokens::OpeningParenthesis>(tokens.front()) and
      std::holds_alternative<tokens::ClosingParenthesis>(tokens.back()))
  {
    return make_tree(std::span(tokens.begin()+1, tokens.end()-1));
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
      std::span(tokens.begin() + 2, tokens.end() - 1));

    if (not non_pth_wrapped_args.has_value())
      return tl::unexpected(non_pth_wrapped_args.error());

    // add the FunctionCallEnd token so we handle it in the loop
    non_pth_wrapped_args->push_back(tokens.end()-1);

    std::vector<SyntaxTree> subnodes;
    auto last_non_coma_token_it = tokens.begin()+2;
    for (auto tokenIt: *non_pth_wrapped_args)
    {
      if (std::holds_alternative<tokens::FunctionArgumentSeparator>(*tokenIt) or
          std::holds_alternative<tokens::FunctionCallEnd>(*tokenIt))
      {
        auto expected_func_argument = make_tree(std::span(last_non_coma_token_it, tokenIt));
        if (not expected_func_argument.has_value())
          return expected_func_argument;
        else subnodes.push_back(std::move(expected_func_argument.value()));
        last_non_coma_token_it = tokenIt+1;
      }
    }

    return FunctionNode(text_token(tokens.front()), std::move(subnodes));
  }

  // there are tokens that are not within parentheses
  else if (not non_pth_enclosed_tokens.empty())
  {
    // we check for operations
    // loop through the expression by increasing operator priority
    // -> because the deepest parts of the syntax tree are to be calculated first
    for (const auto& [op, op_str]: tokens::Operator::operators)
    {
      for (auto tokenIt: non_pth_enclosed_tokens)
      {
        if (std::holds_alternative<tokens::Operator>(*tokenIt) and
            std::get<tokens::Operator>(*tokenIt).name == op_str)
        {
          // we are not within parentheses, and we are at the right operator priority
          if (tokenIt == tokens.begin() or tokenIt+1 == tokens.end())
            return tl::unexpected(ParsingError::unexpected(*tokenIt));

          auto left_hand_side = make_tree(std::span(tokens.begin(), tokenIt));
          if (not left_hand_side.has_value())
            return left_hand_side;

          auto right_hand_side = make_tree(std::span(tokenIt+1, tokens.end()));
          if (not right_hand_side.has_value())
            return right_hand_side;

          return FunctionNode(text_token(*tokenIt),
                              {std::move(left_hand_side.value()),
                               std::move(right_hand_side.value())});
        }
      }
    }
  }

  // if we reach the end of this function, something is not right
  const SubstrInfo substrinfo
    = std::accumulate(tokens.begin(),
                      tokens.end(),
                      substr_info(tokens.front()),
                      [](const SubstrInfo& info, const Token& t1)
                      { return substr_info(t1) + info; });

  return tl::unexpected(ParsingError::unexpected(tokens::Unkown("", substrinfo)));
}
}
