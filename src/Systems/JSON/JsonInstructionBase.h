#ifndef JSONINSTRUCTIONBASE_H
#define JSONINSTRUCTIONBASE_H

#include "JsonInstructionSetRunner.h"

#include <enum.h>

#include <QVariant>
#include <map>
#include <variant>

BETTER_ENUM(EArgumentType, qint32,
            eBool = QVariant::Bool,
            eInt64 = QVariant::LongLong,
            eUInt64 = QVariant::ULongLong,
            eDouble = QVariant::Double,
            eString = QVariant::String,
            eObject = QVariant::UserType,
            eArray = QVariant::List,
            eMap = QVariant::UserType+1)

//----------------------------------------------------------------------------------------
//
struct SInstructionArgumentType;
typedef std::map<QString, SInstructionArgumentType> tInstructionMapType;
typedef std::variant<std::shared_ptr<SInstructionArgumentType> /*can't declare variant with forward declared structs*/,
                     tInstructionMapType, std::false_type> tNestedType;
struct SInstructionArgumentType
{
  SInstructionArgumentType(EArgumentType type) :
    m_type(type) {}
  SInstructionArgumentType(EArgumentType type, const tNestedType& nested) :
    m_type(type), m_nestedType(nested) {}
  EArgumentType m_type;
  tNestedType m_nestedType = std::false_type();
};
inline std::shared_ptr<SInstructionArgumentType>
MakeArgArray(EArgumentType type, const tNestedType& nested = std::false_type())
{
  return std::make_shared<SInstructionArgumentType>(type, nested);
}


struct SInstructionArgumentValue;
typedef std::vector<SInstructionArgumentValue> tInstructionArrayValue;
typedef std::map<QString, SInstructionArgumentValue> tInstructionMapValue;
typedef std::variant<bool, qint64, quint64, double, QString,
                     tInstructionArrayValue, tInstructionMapValue> tInstructionArgumentValue;
struct SInstructionArgumentValue
{
  EArgumentType m_type;
  tInstructionArgumentValue m_value;
};

//----------------------------------------------------------------------------------------
//
namespace detail {
  // generic mapping of unknown enum value to false_type
  template<EArgumentType::_enumerated T> struct arg_value_type : std::false_type {};
  // specialization of type for each enum value
  template<> struct arg_value_type<EArgumentType::eBool> : std::true_type { typedef bool type; };
  template<> struct arg_value_type<EArgumentType::eInt64> : std::true_type { typedef qint64 type; };
  template<> struct arg_value_type<EArgumentType::eUInt64> : std::true_type { typedef quint64 type; };
  template<> struct arg_value_type<EArgumentType::eDouble> : std::true_type { typedef double type; };
  template<> struct arg_value_type<EArgumentType::eString> : std::true_type { typedef QString type; };
  template<> struct arg_value_type<EArgumentType::eObject> : std::true_type
  { typedef QString type; };
  template<> struct arg_value_type<EArgumentType::eArray> : std::true_type
  { typedef tInstructionArrayValue type; };
  template<> struct arg_value_type<EArgumentType::eMap> : std::true_type
  { typedef tInstructionMapValue type; };
}

//----------------------------------------------------------------------------------------
//
inline SInstructionArgumentValue ValuefromVariant(QVariant var, EArgumentType type)
{
  switch (type)
  {
    case EArgumentType::eBool: return { type, var.toBool() };
    case EArgumentType::eInt64: return { type, var.toLongLong() };
    case EArgumentType::eUInt64: return { type, var.toULongLong() };
    case EArgumentType::eDouble: return { type, var.toDouble() };
    case EArgumentType::eString: return { type, var.toString() };
    case EArgumentType::eObject: return { type, var.toString() };
    case EArgumentType::eArray: return { type, tInstructionArrayValue() };
    case EArgumentType::eMap: return { type, tInstructionMapValue() };
    default: return { type, {} };
  }
}

template<EArgumentType::_enumerated T>
std::variant<typename detail::arg_value_type<T>::type, std::false_type>
GetValue(const tInstructionMapValue& from, const QString& sValue)
{
  typedef typename detail::arg_value_type<T>::type retType;
  auto it = from.find(sValue);
  if (from.end() != it)
  {
    if (std::holds_alternative<retType>(it->second.m_value))
    {
      return std::get<retType>(it->second.m_value);
    }
  }
  return std::false_type();
}

template<EArgumentType::_enumerated T>
std::variant<typename detail::arg_value_type<T>::type, std::false_type>
GetValue(const tInstructionArrayValue& from, size_t iValue)
{
  typedef typename detail::arg_value_type<T>::type retType;
  if (0 <= iValue && from.size() > iValue)
  {
    if (std::holds_alternative<retType>(from[iValue].m_value))
    {
      return std::get<retType>(from[iValue].m_value);
    }
  }
  return std::false_type();
}

inline bool HasValue(const tInstructionMapValue& from, const QString& sValue)
{
  auto it = from.find(sValue);
  if (from.end() != it)
  {
    return true;
  }
  return false;
}

template<EArgumentType::_enumerated T>
bool IsOk(const std::variant<typename detail::arg_value_type<T>::type, std::false_type>& value)
{
  return std::holds_alternative<typename detail::arg_value_type<T>::type>(value);
}

//----------------------------------------------------------------------------------------
//
class IJsonInstructionBase
{
public:
  typedef std::variant<SJsonException,
                       SRunRetVal<ENextCommandToCall::eChild>,
                       SRunRetVal<ENextCommandToCall::eSibling>,
                       std::true_type /*Default: dfs traversal of tree*/> tRetVal;

  IJsonInstructionBase() {}
  virtual ~IJsonInstructionBase() {}

  virtual tInstructionMapType& ArgList() = 0;
  virtual IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) = 0;
};

#endif // JSONINSTRUCTIONBASE_H
