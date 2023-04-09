#pragma once

#include <zecalculator/builtin_binary_functions.h>
#include <zecalculator/builtin_unary_functions.h>
#include <zecalculator/function.h>
#include <zecalculator/global_constant.h>
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

  template <class Object>
  class MathObject
  {
  public:
    Object& operator * ()
    {
      return world.get<Object>(id);
    }

    Object* operator -> ()
    {
      return &world.get<Object>(id);
    }

  protected:
    MathObject(MathWorldT& world, size_t id)
      : world(world), id(id) {}

    MathWorldT& world;
    size_t id;
    friend MathWorldT;

    template <class>
    friend class ConstMathObject;
  };


  template <class Object>
  class ConstMathObject
  {
  public:
    const Object& operator * () const
    {
      return world.get<Object>(id);
    }

    const Object* operator -> () const
    {
      return &world.get<Object>(id);
    }

  protected:
    ConstMathObject(MathObject<Object>& math_object)
      : world(math_object.world), id(math_object.id) {}

    ConstMathObject(const MathWorldT& world, size_t id)
      : world(world), id(id) {}

    const MathWorldT& world;
    size_t id;
    friend MathWorldT;

    template <class>
    friend class MathObject;
  };

  class UnregisteredObject {};

  /// @brief type used when looking up a match object with a name at runtime
  using DynMathObject = std::variant<UnregisteredObject, MathObject<MathObjectType>...>;

  /// @brief const version of the one above
  using ConstDynMathObject = std::variant<UnregisteredObject, ConstMathObject<MathObjectType>...>;

  MathWorldT() = default;

  template <class ObjectType1, size_t size1, class... ObjectTypeN, size_t... sizeN>
  MathWorldT(
    const std::array<std::pair<std::string_view, ObjectType1>, size1>& objects1,
    const std::array<std::pair<std::string_view, ObjectTypeN>, sizeN>&... objectsN)
    : MathWorldT(objectsN...)
  {
    for(auto [name, obj]: objects1)
      add<ObjectType1>(name, obj);
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
    if (contains(name))
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
    if (contains(name))
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

class MathWorld: public MathWorldT<CppUnaryFunction, CppBinaryFunction, GlobalConstant, Function>
{
  using Parent = MathWorldT<CppUnaryFunction, CppBinaryFunction, GlobalConstant, Function>;
public:
  /// @brief default world that contains the usual functions (cos, sin ...)
  static const MathWorld default_world;

  using Parent::MathWorldT;

  MathWorld() : MathWorld(default_world) {}

};

extern MathWorld global_world;

}
