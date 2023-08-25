#pragma once

#include <zecalculator/math_objects/builtin_binary_functions.h>
#include <zecalculator/math_objects/builtin_unary_functions.h>
#include <zecalculator/math_objects/function.h>
#include <zecalculator/math_objects/global_constant.h>
#include <zecalculator/math_objects/global_variable.h>
#include <zecalculator/math_objects/sequence.h>
#include <zecalculator/utils/name_map.h>
#include <zecalculator/utils/optional_ref.h>
#include <zecalculator/utils/slotted_vector.h>
#include <zecalculator/utils/utils.h>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

namespace zc {

struct NameError
{
  enum Type {ALREADY_TAKEN, INVALID_FORMAT};

  static NameError already_taken(std::string_view name)
  {
    return NameError{.type = ALREADY_TAKEN, .name = std::string(name)};
  }

  static NameError invalid_format(std::string_view name)
  {
    return NameError{.type = INVALID_FORMAT, .name = std::string(name)};
  }

  Type type;
  std::string name;
};

template <class... MathObjectType>
class MathWorldT;

using MathWorld = MathWorldT<CppUnaryFunction, CppBinaryFunction, GlobalConstant, Function, GlobalVariable, Sequence>;

template <class... MathObjectType>
class MathWorldT
{
public:

  template <class Object, bool is_const>
  class MathObjectT
  {
    using math_world_t = std::conditional_t<is_const, const MathWorldT&, MathWorldT&>;

  public:
    math_world_t world;
    size_t id;

    MathObjectT(const MathObjectT& obj) = default;
    MathObjectT(MathObjectT&& obj) = default;

    MathObjectT<Object, true> to_const() const
      requires (not is_const)
    {
      return MathObjectT<Object, true>(*this);
    }

    Object& operator * () requires (not is_const)
    {
      return world.template get<Object>(id);
    }

    Object* operator -> () requires (not is_const)
    {
      return &world.template get<Object>(id);
    }

    const Object& operator * () const
    {
      return world.template get<Object>(id);
    }

    const Object* operator -> () const
    {
      return &world.template get<Object>(id);
    }

    auto operator()(const std::vector<double>& arg) const
      requires std::is_invocable_v<Object, std::vector<double>, math_world_t>
    {
      return (**this)(arg, world);
    }

    auto operator()(double arg) const
      requires std::is_invocable_v<Object, double, math_world_t>
    {
      return (**this)(arg, world);
    }

    auto operator()() const
      requires std::is_invocable_v<Object, math_world_t>
    {
      return (**this)(world);
    }

  protected:
    MathObjectT(const MathObjectT<Object, false>& obj) requires (is_const)
      : world(obj.world), id(obj.id) {}

    MathObjectT(math_world_t world, size_t id)
      : world(world), id(id) {}

    friend MathWorldT;
  };

  template <class Object>
  using MathObject = MathObjectT<Object, false>;

  template <class Object>
  using ConstMathObject = MathObjectT<Object, true>;

  class UnregisteredObject {};

  /// @brief type used when looking up a match object with a name at runtime
  using DynMathObject = std::variant<UnregisteredObject, MathObject<MathObjectType>...>;

  /// @brief const version of the one above
  using ConstDynMathObject = std::variant<UnregisteredObject, ConstMathObject<MathObjectType>...>;

  /// @brief default constructor when this class is used outside of what it's been planned for
  MathWorldT() requires (not std::is_same_v<MathWorldT, MathWorld>) = default;

  /// @brief default constructor that defines the usual functions and global constants
  MathWorldT()
    requires(std::is_same_v<MathWorldT, MathWorld>)
    : MathWorldT(builtin_unary_functions, builtin_binary_functions, builtin_global_variables){};

  template <class ObjectType1, size_t size1, class... ObjectTypeN, size_t... sizeN>
  MathWorldT(
    const std::array<std::pair<std::string_view, ObjectType1>, size1>& objects1,
    const std::array<std::pair<std::string_view, ObjectTypeN>, sizeN>&... objectsN)
    : MathWorldT(objectsN...)
  {
    for(auto [name, obj]: objects1)
      add<ObjectType1>(name, obj);
  }

  template <class ObjectType, size_t size>
  MathWorldT(const std::array<std::pair<std::string_view, ObjectType>, size>& objects)
  {
    for(auto [name, obj]: objects)
      add<ObjectType>(name, obj);
  }

  /// @brief get object from name, the underlying type is to be dynamically resolved at runtime
  /// @note const version
  ConstDynMathObject get(std::string_view name) const
  {
    auto it = inventory.find(name);
    return it != inventory.end() ? to_const(it->second) : ConstDynMathObject();
  }

  /// @brief get object from name, the underlying type is to be dynamically resolved at runtime
  DynMathObject get(std::string_view name)
  {
    auto it = inventory.find(name);
    return it != inventory.end() ? it->second : DynMathObject();
  }

  /// @brief get object 'ObjectType' from name, if it exists
  template <class ObjectType>
  std::optional<MathObject<ObjectType>> get(std::string_view name)
  {
    DynMathObject dyn_obj = get(name);
    if (std::holds_alternative<MathObject<ObjectType>>(dyn_obj))
      return std::get<MathObject<ObjectType>>(dyn_obj);
    else return {};
  }

  /// @brief get object 'ObjectType' from name, if it exists
  /// @note const version
  template <class ObjectType>
  std::optional<ConstMathObject<ObjectType>> get(std::string_view name) const
  {
    ConstDynMathObject dyn_obj = get(name);
    if (std::holds_alternative<ConstMathObject<ObjectType>>(dyn_obj))
      return std::get<ConstMathObject<ObjectType>>(dyn_obj);
    else return {};
  }

  /// @brief get object 'ObjectType' from id
  /// @note ids are not unique, they live in different sets between ObjectTypes
  template <class ObjectType>
  ObjectType& get(size_t id)
  {
    return std::get<SlottedVector<ObjectType>>(math_objects).at(id);
  }

  /// @brief get object 'ObjectType' from id
  /// @note const version
  template <class ObjectType>
  const ObjectType& get(size_t id) const
  {
    return std::get<SlottedVector<ObjectType>>(math_objects).at(id);
  }

  /// @brief moves ObjectType 'object' into the world, under the name 'name'
  /// @note throws if the name is already taken, leaves the world unchanged.
  template <class ObjectType>
  tl::expected<MathObject<ObjectType>, NameError> add(std::string_view name, ObjectType object)
  {
    if (not parsing::is_valid_name(name))
      return tl::unexpected(NameError::invalid_format(name));
    else if (contains(name))
      return tl::unexpected(NameError::already_taken(name));

    size_t id = std::get<SlottedVector<ObjectType>>(math_objects).push(std::move(object));
    auto world_object = MathObject<ObjectType>(*this, id);
    inventory.insert({std::string(name), world_object});
    return world_object;
  }

  /// @brief default constructs an ObjectType in the world, under the name 'name'
  /// @note throws if the name is already taken, leaves the world unchanged.
  template <class ObjectType>
  tl::expected<MathObject<ObjectType>, NameError> add(std::string_view name)
  {
    if (not parsing::is_valid_name(name))
      return tl::unexpected(NameError::invalid_format(name));
    else if (contains(name))
      return tl::unexpected(NameError::already_taken(name));

    size_t id = std::get<SlottedVector<ObjectType>>(math_objects).push(ObjectType());
    auto world_object = MathObject<ObjectType>(*this, id);
    inventory.insert({std::string(name), world_object});
    return world_object;
  }

  /// @brief says if an object with the given name exists within the world
  bool contains(std::string_view name) const
  {
    return inventory.find(name) != inventory.end();
  }

  /// @brief maximum recursion depth to reach before returning an error
  size_t max_recursion_depth = 100;


protected:

  /// @brief converts a DynMathObject to a ConstDynMathObject
  ConstDynMathObject to_const(DynMathObject obj) const
  {
    return std::visit(
      overloaded{
        [](UnregisteredObject) -> ConstDynMathObject { return UnregisteredObject(); },
        [](auto&& val) -> ConstDynMathObject { return val.to_const(); }
      },
      obj);
  }

  /// @brief maps an object name to its type and ID (index within the container that holds it)
  name_map<DynMathObject> inventory;

  std::tuple<SlottedVector<MathObjectType>...> math_objects;

};

}
