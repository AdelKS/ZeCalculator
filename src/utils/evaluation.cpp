#include <zecalculator/utils/evaluation.h>
#include <zecalculator/utils/utils.h>

namespace zc {

tl::expected<double, EvaluationError> evaluate(const SyntaxTree& tree,
                                               const name_map<double>& input_vars,
                                               const MathWorld& world)
{
  using ReturnType = tl::expected<double, EvaluationError>;
  return std::visit(
    [&](auto&& node) -> ReturnType {

      using T = std::remove_cvref_t<decltype(node)>;

      if constexpr (std::is_same_v<T, std::monostate>)
      {
        return tl::unexpected(EvaluationError::empty_expression());
      }
      else if constexpr (std::is_same_v<T, FunctionNode>)
      {
        auto math_obj = world.get_math_object(node.name);

        return std::visit(
          [&](auto&& function) -> ReturnType {

            std::vector<double> evaluations;
            for (const auto& subnode: node.subnodes)
            {
              auto eval = evaluate(subnode, input_vars, world);
              if (eval) [[likely]]
                evaluations.push_back(*eval);
              else [[unlikely]]
                return eval;
            }

            using F = std::remove_cvref_t<decltype(function)>;
            if constexpr (std::is_convertible_v<F, CppUnaryFunction>)
            {
              if (evaluations.size() != 1)
                return tl::unexpected(EvaluationError::mismatched_fun_args(node));
              else return function(evaluations.front());
            }
            else if constexpr (std::is_convertible_v<F, CppBinaryFunction>)
            {
              if (evaluations.size() != 2)
                return tl::unexpected(EvaluationError::mismatched_fun_args(node));
              else return function(evaluations.front(), evaluations.back());
            }
            else if constexpr (std::is_convertible_v<F, Function>)
            {
              const Function& f = function;
//              std::cout << "Evaluating zc function: " << node.name << std::endl;
              if (evaluations.size() != f.argument_size())
                return tl::unexpected(EvaluationError::mismatched_fun_args(node));
              else
              {
                ReturnType eval = f(evaluations, world);
                if (not bool(eval)) [[unlikely]]
                  return tl::unexpected(EvaluationError::calling_invalid_function(node));
                else [[likely]]
                  return eval;
              }
            }
            else return tl::unexpected(EvaluationError::not_implemented(node));

          },
          math_obj);
      }

      else if constexpr (std::is_same_v<T, VariableNode>)
      {
        auto it = input_vars.find(node.name);
        if (it != input_vars.end())
          return it->second;
        else
        {
          auto var_cref_w = world.get_global_constant(node.name);
          if(var_cref_w)
            return var_cref_w->get().value;
          else return tl::unexpected(EvaluationError::undefined_variable(node));
        }
      }

      else if constexpr (std::is_same_v<T, NumberNode>)
        return node.value;

      else return tl::unexpected(EvaluationError::not_implemented(node));
    },
    tree);
}

}
