#include <zecalculator/utils/evaluation.h>
#include <zecalculator/utils/utils.h>

namespace zc {

tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree, const MathWorld& world)
{
  using ReturnType = tl::expected<double, EvaluationError>;
  return std::visit(
    [&](auto&& node) -> ReturnType {

      using T = std::remove_cvref_t<decltype(node)>;

      if constexpr (std::is_same_v<T, FunctionNode>)
      {
        auto math_obj = mathWorld.get_math_object(node.name);

        return std::visit(
          [&](auto&& function) -> ReturnType {

            using F = std::remove_cvref_t<decltype(function)>;
            if constexpr (std::is_same_v<F, BuiltinFunction>)
            {
              if (node.subnodes.size() != 1)
                return tl::unexpected(EvaluationError::mismatched_fun_args(node));

              auto evaluation = evaluate(node.subnodes.front(), world);
              if (evaluation)
                return function(evaluation.value());
              else return evaluation;
            }
            else return tl::unexpected(EvaluationError::not_implemented(node));

          },
          math_obj);
      }

      else if constexpr (std::is_same_v<T, VariableNode>)
        return tl::unexpected(EvaluationError::not_implemented(node));

      else if constexpr (std::is_same_v<T, NumberNode>)
        return node.value;

      else return tl::unexpected(EvaluationError::not_implemented(node));
    },
    tree);
}

}
