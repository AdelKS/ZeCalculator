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
        auto math_obj = world.get(node.name);

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
            if constexpr (std::is_same_v<F, MathWorld::ConstMathObject<CppUnaryFunction>>)
            {
              if (evaluations.size() != 1)
                return tl::unexpected(EvaluationError::mismatched_fun_args(node));
              else return (*function)(evaluations.front());
            }
            else if constexpr (std::is_convertible_v<F, MathWorld::ConstMathObject<CppBinaryFunction>>)
            {
              if (evaluations.size() != 2)
                return tl::unexpected(EvaluationError::mismatched_fun_args(node));
              else return (*function)(evaluations.front(), evaluations.back());
            }
            else if constexpr (std::is_convertible_v<F, MathWorld::ConstMathObject<Function>>)
            {
//              std::cout << "Evaluating zc function: " << node.name << std::endl;
              if (evaluations.size() != function->argument_size())
                return tl::unexpected(EvaluationError::mismatched_fun_args(node));
              else
              {
                ReturnType eval = function(evaluations);
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
          using GlobalConstantWrapper = MathWorld::ConstMathObject<GlobalConstant>;
          using GlobalVariableWrapper = MathWorld::ConstMathObject<GlobalVariable>;

          auto math_object = world.get(node.name);

          if (std::holds_alternative<GlobalConstantWrapper>(math_object))
            return std::get<GlobalConstantWrapper>(math_object)->value;
          else if (std::holds_alternative<GlobalVariableWrapper>(math_object))
            return std::get<GlobalVariableWrapper>(math_object)();
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
