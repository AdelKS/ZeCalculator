#pragma once

#include <zecalculator/math_objects/decl/internal/eq_object.h>
#include <zecalculator/parsing/impl/parser.h>

namespace zc {
namespace internal {

template <parsing::Type type>
tl::expected<MathObjectsVariant<type>, Error> EqObject::to_expected_unbound() const
{
  switch(cat)
  {
    case EqObject::FUNCTION:
      return Function<type>(lhs.args_num());
    case EqObject::SEQUENCE:
      return Sequence<type>();
    case EqObject::GLOBAL_CONSTANT:
      return GlobalConstant(name, rhs.number_data().value);
    default: [[unlikely]]
      throw std::runtime_error("Bug in ZeCalculator");
  }
}

template <parsing::Type type>
tl::expected<MathObjectsVariant<type>, Error>
  EqObject::to_expected(const zc::MathWorld<type>& mathworld) const
{
  auto get_final_representation = [&](const std::string& eq, const parsing::AST& ast)
  {
    auto final_ast = parsing::mark_input_vars{this->var_names}(ast);
    if constexpr (type == parsing::Type::FAST)
      return parsing::make_fast<type>{eq, mathworld}(final_ast);
    else
      return parsing::make_fast<type>{eq, mathworld}(final_ast).transform(parsing::make_RPN);
  };

  switch(cat)
  {
    case EqObject::FUNCTION:

      if (auto exp_rhs = get_final_representation(equation, rhs))
        return Function<type>(name, equation, lhs.args_num(), std::move(*exp_rhs));
      else
        return tl::unexpected(std::move(exp_rhs.error()));

    case EqObject::SEQUENCE:
    {
      std::vector<const parsing::AST*> seq_ast_values;

      if (rhs.is_func() and rhs.func_data().type == parsing::AST::Func::SEPARATOR)
      {
        // sequence defined with first values
        seq_ast_values.reserve(rhs.func_data().subnodes.size());
        std::ranges::transform(rhs.func_data().subnodes,
                                std::back_inserter(seq_ast_values),
                                [](auto&& val){ return &val; });
      }
      // sequence defined without first values
      else seq_ast_values.push_back(&rhs);

      std::vector<parsing::Parsing<type>> values;
      values.reserve(seq_ast_values.size());
      for (const parsing::AST* ast: seq_ast_values)
        if (auto exp_rhs = get_final_representation(equation, *ast))
          values.push_back(std::move(*exp_rhs));
        else
          return tl::unexpected(std::move(exp_rhs.error()));

      return Sequence<type>(name, equation, std::move(values));
    }
    case EqObject::GLOBAL_CONSTANT:
      return GlobalConstant(name, rhs.number_data().value);

    default: [[unlikely]]
      throw std::runtime_error("Bug in ZeCalculator");
  }
}

inline deps::Deps EqObject::direct_dependencies() const
{
  return zc::parsing::direct_dependencies(zc::parsing::mark_input_vars{var_names}(rhs));
}

} // namespace internal
} // namespace zc
