#ifndef JSONINSTRUCTIONTYPES_H
#define JSONINSTRUCTIONTYPES_H

#include <enum.h>

#include <QString>
#include <QVariant>

#include <any>
#include <map>
#include <memory>
#include <type_traits>
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

Q_DECLARE_METATYPE(tInstructionMapValue)

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
struct SJsonException
{
  QString m_sException;
  QString m_sToken;
  QString m_sCommand;
  qint32  m_iLineNr;
  qint32  m_iColumn;
};

enum class ERunerMode
{
  eAutoRunAll,  // run all commands blocking
  eRunOne       // return after each command and advance runner state
};

Q_DECLARE_METATYPE(ERunerMode)

//----------------------------------------------------------------------------------------
//
enum class ENextCommandToCall
{
  eChild = 0,
  eSibling,
  eForkThis,
  eFinish
};

template<ENextCommandToCall M> struct SRunRetVal : std::false_type {};
template<> struct SRunRetVal<ENextCommandToCall::eChild>
{
  SRunRetVal(qint32 iIndex) : m_iIndex(iIndex) {}
  ENextCommandToCall m_type = ENextCommandToCall::eChild;
  qint32             m_iIndex;
};
template<> struct SRunRetVal<ENextCommandToCall::eSibling>
{
  SRunRetVal() {}
  ENextCommandToCall m_type = ENextCommandToCall::eSibling;
};
template<> struct SRunRetVal<ENextCommandToCall::eForkThis>
{
  SRunRetVal(const tInstructionMapValue& args, const QString& sName) :
    m_args(args), m_sName(sName) {}
  ENextCommandToCall   m_type = ENextCommandToCall::eForkThis;
  tInstructionMapValue m_args; // arguments for the this call in new worker thread
  QString              m_sName; // New commands-Fork name
};
template<> struct SRunRetVal<ENextCommandToCall::eFinish>
{
  SRunRetVal(std::any retVal) : m_retVal(retVal) {}
  ENextCommandToCall m_type = ENextCommandToCall::eFinish;
  std::any           m_retVal;
};


#endif // JSONINSTRUCTIONTYPES_H
