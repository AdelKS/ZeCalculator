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

#include <zecalculator/utils/syntax_tree.h>
#include <zecalculator/utils/parser.h>
#include <zecalculator/utils/error.h>

namespace zc {

tl::expected<
  std::vector<std::span<Token>::iterator>,
  Error
> get_non_pth_enclosed_tokens(std::span<Token> tokens)
{
  std::vector<std::span<Token>::iterator> non_pth_enclosed_tokens;

  enum : bool { FUNCTION_CALL_PTH, NORMAL_PTH};
  std::stack<bool> last_opened_pth;

  // search for parentheses
  for (auto tokenIt = tokens.begin() ; tokenIt != tokens.end() ; tokenIt++)
  {
    if (tokenIt->type == Token::OPENING_PARENTHESIS)
    {
      last_opened_pth.push(NORMAL_PTH);
    }
    else if (tokenIt->type == Token::FUNCTION_CALL_START)
    {
      last_opened_pth.push(FUNCTION_CALL_PTH);
    }
    else if (tokenIt->type == Token::FUNCTION_CALL_END)
    {
      if (not last_opened_pth.empty() and last_opened_pth.top() == FUNCTION_CALL_PTH)
        last_opened_pth.pop();
      else return tl::unexpected(Error::unexpected(Token::FUNCTION_CALL_END, tokenIt->str_v));
    }
    else if (tokenIt->type == Token::CLOSING_PARENTHESIS)
    {
      if (not last_opened_pth.empty() and last_opened_pth.top() == NORMAL_PTH)
        last_opened_pth.pop();
      else return tl::unexpected(Error::unexpected(Token::CLOSING_PARENTHESIS, tokenIt->str_v));
    }
    // if not a parenthesis, and the token is not enclosed within parentheses, push it
    else if (last_opened_pth.empty())
      non_pth_enclosed_tokens.push_back(tokenIt);
  }

  return non_pth_enclosed_tokens;
}

/// @brief when the input range of str views are a split of a bigger str view
///        returns the str view
/// @param str_views: container of string_views that are a split of a bigger str_view
/// @returns the bigger string view
template <std::ranges::viewable_range Range>
requires std::is_convertible_v<std::ranges::range_value_t<Range>, std::string_view>
std::optional<std::string_view> concatenate(const Range& str_views)
{
  // check that all string views are just splits of global string view
  std::string_view previous;
  size_t final_size = 0;

  for (std::string_view view: str_views)
  {
    // set the 'previous' str_view if it's not set
    if (previous.data() == nullptr)
      previous = view;

    // if the current view has its data  not start at the end of
    // the previous, we have an issue and we return an empty optional
    else if (previous.data() + previous.size() != view.data())
      return std::optional<std::string_view>();

    previous = view;
    final_size += view.size();
  }

  return std::optional<std::string_view>(std::in_place, (*str_views.begin()).data(), final_size);
}

tl::expected<SyntaxTree, Error> make_tree(const std::span<Token> tokens)
{
  // when there's only a single token, it can only be number of a variable
  if (tokens.size() == 1)
  {
    const Token& single_token = tokens.back();
    if (single_token.type == Token::NUMBER or
        single_token.type == Token::VARIABLE)
    {
      const double* dbl_value = std::get_if<double>(&single_token.type_value);
      return SyntaxTree {
        .type = SyntaxTree::Type(single_token.type),
        .str = std::string(single_token.str_v),
        .value = dbl_value ? *dbl_value : std::optional<double>()};
    }
    else return tl::unexpected(Error::unexpected(single_token.type, single_token.str_v));
  }

  auto expected_non_pth_wrapped_tokens = get_non_pth_enclosed_tokens(tokens);
  if (not expected_non_pth_wrapped_tokens.has_value())
    return tl::unexpected(expected_non_pth_wrapped_tokens.error());

  const auto& non_pth_enclosed_tokens = expected_non_pth_wrapped_tokens.value();

  // experssion of the type "(...)"
  if (non_pth_enclosed_tokens.empty() and tokens.size() > 2 and
      tokens.begin()->type == Token::OPENING_PARENTHESIS and
      tokens.rbegin()->type == Token::CLOSING_PARENTHESIS)
  {
    return make_tree(std::span(tokens.begin()+1, tokens.end()-1));
  }

  // expression of the type "function(...)"
  else if (non_pth_enclosed_tokens.size() == 1 and tokens.size() > 3 and
           tokens.begin()->type == Token::FUNCTION and
           (tokens.begin()+1)->type == Token::FUNCTION_CALL_START and
           tokens.rbegin()->type == Token::FUNCTION_CALL_END)
  {
    auto expected_func_argument = make_tree(std::span(tokens.begin()+2, tokens.end()-1));
    if (not expected_func_argument.has_value())
      return expected_func_argument;

    return SyntaxTree {
      .type = SyntaxTree::FUNCTION,
      .str = std::string(tokens.begin()->str_v),
      .subnodes = {std::move(expected_func_argument.value())}
    };
  }

  // there are tokens that are not within parentheses
  else if (not non_pth_enclosed_tokens.empty())
  {
    // we check for operations
    // loop through the expression by increasing operator priority
    // -> because the deepest parts of the syntax tree are to be calculated first
    for (char op: Token::operators)
    {
      for (auto tokenIt: non_pth_enclosed_tokens)
      {
        if (tokenIt->type == Token::OPERATOR and
            std::get<char>(tokenIt->type_value) == op)
        {
          // we are not within parentheses, and we are at the right operator priority
          if (tokenIt == tokens.begin() or tokenIt+1 == tokens.end())
            return tl::unexpected(Error::unexpected(Token::OPERATOR, tokenIt->str_v));

          auto left_hand_side = make_tree(std::span(tokens.begin(), tokenIt));
          if (not left_hand_side.has_value())
            return left_hand_side;

          auto right_hand_side = make_tree(std::span(tokenIt+1, tokens.end()));
          if (not right_hand_side.has_value())
            return right_hand_side;

          return SyntaxTree {
            .type = SyntaxTree::FUNCTION,
            .str = std::string(tokenIt->str_v),
            .subnodes = {std::move(std::move(left_hand_side.value())),
                         std::move(right_hand_side.value())}
          };
        }
      }
    }
  }

  // if we reach the end of this function, something is not right
  return tl::unexpected(
          Error::unexpected(
            Token::UNKNOWN,
            concatenate(tokens | std::views::transform(&Token::str_v)).value()
         ));
}

}
