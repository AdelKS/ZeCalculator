#pragma once

#include <zecalculator/builtin_binary_functions.h>
#include <zecalculator/builtin_unary_functions.h>
#include <zecalculator/function.h>
#include <zecalculator/global_constant.h>
#include <zecalculator/global_variable.h>
#include <zecalculator/utils/name_map.h>
#include <zecalculator/utils/optional_ref.h>
#include <zecalculator/utils/slotted_vector.h>
#include <zecalculator/utils/utils.h>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

namespace zc {

template <class... MathObjectType>
class MathWorldT;

using MathWorld = MathWorldT<CppUnaryFunction, CppBinaryFunction, GlobalConstant, Function, GlobalVariable>;

template <class... MathObjectType>
class MathWorldT
{
public:
  class name_already_taken: public std::runtime_error
  {
  public:
    name_already_taken(std::string name)
      : std::runtime_error("name '" + name + "' already taken"),
        name(name) {};

    std::string name;
  };

  class invalid_name_format: public std::runtime_error
  {
  public:
    invalid_name_format(std::string name)
      : std::runtime_error("name '" + name + "' has an invalid format"),
        name(name) {};

    std::string name;
  };

  template <class Object, bool is_const>
  class MathObjectT
  {
    using math_world_t = std::conditional_t<is_const, const MathWorldT&, MathWorldT&>;

  public:
    MathObjectT(const MathObjectT& obj) = default;
    MathObjectT(MathObjectT&& obj) = default;

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

    math_world_t world;
    size_t id;
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
  MathObject<ObjectType> add(std::string_view name, ObjectType object)
  {
    if (not is_valid_name(name))
      throw invalid_name_format(std::string(name));
    else if (contains(name))
      throw name_already_taken(std::string(name));

    size_t id = std::get<SlottedVector<ObjectType>>(math_objects).push(std::move(object));
    auto world_object = MathObject<ObjectType>(*this, id);
    inventory.insert({std::string(name), world_object});
    return world_object;
  }

  /// @brief default constructs an ObjectType in the world, under the name 'name'
  /// @note throws if the name is already taken, leaves the world unchanged.
  template <class ObjectType>
  MathObject<ObjectType> add(std::string_view name)
  {
    if (not is_valid_name(name))
      throw invalid_name_format(std::string(name));
    else if (contains(name))
      throw name_already_taken(std::string(name));

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

protected:

  /// @brief converts a DynMathObject to a ConstDynMathObject
  ConstDynMathObject to_const(DynMathObject obj) const
  {
    return std::visit(
      overloaded{
        [](UnregisteredObject) -> ConstDynMathObject { return UnregisteredObject(); },
        [](auto&& val) -> ConstDynMathObject { return ConstMathObject(val); }
      },
      obj);
  }

  /// @brief maps an object name to its type and ID (index within the container that holds it)
  name_map<DynMathObject> inventory;

  std::tuple<SlottedVector<MathObjectType>...> math_objects;

};

}
