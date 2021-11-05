#include "JsonInstructionSetParser.h"
#include "JsonInstructionSetRunner.h"
#include "JsonInstructionSetSaxParser.h"
#include "JsonInstructionNode.h"

#include <nlohmann/json-schema.hpp>

#include <QDebug>
#include <set>
#include <stack>
#include <string>
#include <tuple>

class CJsonInstructionSetRunnerPrivate;

namespace  {
  const char* c_sCommandsNode = "commands";

  void InstructionSetFormatChecker(const std::string& format, const std::string& value)
  {
    Q_UNUSED(value)
    if (format == "something")
    {
      //if (!check_value_for_something(value))
      //  throw std::invalid_argument("value is not a good something");
    }
    else
    {
      //throw std::logic_error("Don't know how to validate " + format);
    }
  }


  //--------------------------------------------------------------------------------------
  //
  /* json-parse the people - with custom error handler */
  class CustomErrorHandler : public nlohmann::json_schema::basic_error_handler
  {
    public:
      void error(const nlohmann::json_pointer<nlohmann::basic_json<>> &pointer,
                 const nlohmann::json &instance,
                 const std::string &message) override
      {
          nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
          sError = "Validation of json failed:" + QString::fromStdString(pointer.to_string())
                   + ":" + QString::fromStdString(message);
          qWarning() << sError;
      }

      QString sError = QString();
  };

  //--------------------------------------------------------------------------------------
  //
  SJsonException ExceptionToStruct(const QString& sErr)
  {
    QRegExp rxLine("line ([0-9]*)");
    QRegExp rxColumn("column ([0-9]*)");
    SJsonException ret;
    ret.m_sException = sErr;
    qint32 iPos = 0;
    if ((iPos = rxLine.indexIn(sErr, iPos)) != -1)
    {
      ret.m_iLineNr = rxLine.cap(1).toInt();
    }
    iPos = 0;
    if ((iPos = rxColumn.indexIn(sErr, iPos)) != -1)
    {
      ret.m_iColumn = rxLine.cap(1).toInt();
    }
    return ret;
  }

  SJsonException ExceptionToStruct(const std::exception& e)
  {
    return ExceptionToStruct(e.what());
  }
}

//----------------------------------------------------------------------------------------
// CJsonInstructionSetRunnerPrivate class definition
//
class CJsonInstructionSetRunnerPrivate
{
  friend class CJsonInstructionSetParserPrivate;
  friend class CJSonSaxParser;

public:
  CJsonInstructionSetRunnerPrivate() :
    m_vspBuiltCommands(),
    m_spValidationErrorHandler(std::make_unique<CustomErrorHandler>()),
    m_validator(nullptr, InstructionSetFormatChecker, nullptr),
    m_jsonBaseSchema(),
    m_jsonParsed(),
    m_sJson(),
    m_parseError(),
    m_bValidationOk(false),
    m_spNextNode(nullptr)
  {
  }

  ~CJsonInstructionSetRunnerPrivate()
  {
  }

  //--------------------------------------------------------------------------------------
  //
  SJsonException ParseError() const
  {
    return m_parseError;
  }

  //--------------------------------------------------------------------------------------
  //
  bool Validate(const std::string& json)
  {
    if (m_jsonBaseSchema.is_null())
    {
      m_parseError = SJsonException();
      m_bValidationOk = true;
      return m_bValidationOk;
    }

    // parse json with default parser
    try
    {
      m_jsonParsed = nlohmann::json::parse(json);
    }
    catch (const std::exception &e)
    {
      m_parseError = ExceptionToStruct(e);
      return false;
    }

    // set schema
    try
    {
      m_validator.set_root_schema(m_jsonBaseSchema);
    }
    catch (const std::exception &e)
    {
      m_parseError = ExceptionToStruct(e);
      m_bValidationOk = false;
    }

    // validate the document - uses a custom error-handler, because we don't like
    // to throw around things
    m_validator.validate(m_jsonParsed, *m_spValidationErrorHandler);
    if (static_cast<bool>(*m_spValidationErrorHandler))
    {
      m_parseError = ExceptionToStruct(m_spValidationErrorHandler->sError);
      m_bValidationOk = false;
    }
    else
    {
      m_parseError = SJsonException();
      m_bValidationOk = true;
    }

    return m_bValidationOk;
  }

  //--------------------------------------------------------------------------------------
  //
  CJsonInstructionSetRunner::tRetVal CallNextCommand(ERunerMode runMode)
  {
    auto fnCallImpl = [this]() -> CJsonInstructionSetRunner::tRetVal {
      IJsonInstructionBase::tRetVal vRetVal = CallCommand();
      if (std::holds_alternative<SJsonException>(vRetVal))
      {
        return std::get<SJsonException>(vRetVal);
      }
      else if (std::holds_alternative<SRunRetVal<ENextCommandToCall::eChild>>(vRetVal))
      {
        auto& ret = std::get<SRunRetVal<ENextCommandToCall::eChild>>(vRetVal);
        return NextCommand(ret.m_type, ret.m_iIndex);
      }
      else if (std::holds_alternative<SRunRetVal<ENextCommandToCall::eSibling>>(vRetVal))
      {
        auto& ret = std::get<SRunRetVal<ENextCommandToCall::eSibling>>(vRetVal);
        return NextCommand(ret.m_type);
      }
      else return NextCommand(ENextCommandToCall::eChild, 0);
    };


    if (ERunerMode::eAutoRunAll == runMode)
    {
      while (nullptr != m_spNextNode)
      {
        CJsonInstructionSetRunner::tRetVal retVal = fnCallImpl();
        if (std::holds_alternative<SJsonException>(retVal)) { return retVal; }
      }
      return std::true_type();
    }
    else if (ERunerMode::eRunOne == runMode)
    {
      if (nullptr != m_spNextNode)
      {
        return fnCallImpl();
      }
    }

    // nothing to run
    return std::true_type();
  }

  //--------------------------------------------------------------------------------------
  //
  CJsonInstructionSetRunner::tRetVal Run(const QString& sInstructionSet, ERunerMode runMode)
  {
    if (!m_bValidationOk)
    { return SJsonException{"Parse error", "", "", 0, 0}; }

    m_spNextNode = nullptr;

    // validation success, start actually running the commands
    auto it = std::find_if(m_vspBuiltCommands.begin(), m_vspBuiltCommands.end(),
                           [&sInstructionSet](const std::pair<QString, std::shared_ptr<SJsonInstructionNode>>& pair){
      return pair.first == sInstructionSet;
    });
    if (m_vspBuiltCommands.end() != it)
    {
      if (static_cast<qint32>(it->second->m_spChildren.size()) > 0)
      {
        m_spNextNode =
            *it->second->m_spChildren.begin();
        return CallNextCommand(runMode);
      }
    }

    return
        SJsonException{"Can not run instruction set " + sInstructionSet, sInstructionSet, sInstructionSet, 0, 0};
  }


protected:
  //--------------------------------------------------------------------------------------
  //
  IJsonInstructionBase::tRetVal CallCommand()
  {
    if (nullptr != m_spNextNode)
    {
      if (auto spCommand = m_spNextNode->m_wpCommand.lock())
      {
        IJsonInstructionBase::tRetVal retVal =
            spCommand->Call(m_spNextNode->m_actualArgs);
        return retVal;
      }
    }
    return SJsonException{"Internal error.", "", "", 0, 0};
  }

  //--------------------------------------------------------------------------------------
  //
  bool NextCommand(ENextCommandToCall nextCmd, qint32 iIndex = 0)
  {
    if (nullptr != m_spNextNode)
    {
      // dfs
      if (m_spNextNode->m_spChildren.size() > 0 &&
          ENextCommandToCall::eChild == nextCmd)
      {
        iIndex = std::min(static_cast<qint32>(m_spNextNode->m_spChildren.size()-1),
                          std::max(0, iIndex));
        m_spNextNode = *std::next(m_spNextNode->m_spChildren.begin(), iIndex);
        return true;
      }
      else
      {
        auto spParent = m_spNextNode->m_wpParent.lock();
        while (nullptr != spParent)
        {
          auto it =
            std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), m_spNextNode);
          qint32 iIndex = std::distance(spParent->m_spChildren.begin(), it);
          if (static_cast<qint32>(spParent->m_spChildren.size()) > iIndex+1)
          {
            m_spNextNode = spParent->m_spChildren[static_cast<size_t>(iIndex+1)];
            return true;
          }
          else
          {
            // go up one
            m_spNextNode = spParent;
            spParent = spParent->m_wpParent.lock();
          }
        }
      }
    }
    // nothing found -> reset since all commands have run
    m_spNextNode = nullptr;
    return false;
  }

  std::vector<std::pair<QString, std::shared_ptr<SJsonInstructionNode>>>
                                                          m_vspBuiltCommands;
  std::unique_ptr<CustomErrorHandler>                     m_spValidationErrorHandler;
  nlohmann::json_schema::json_validator                   m_validator;
  nlohmann::json                                          m_jsonBaseSchema;
  nlohmann::json                                          m_jsonParsed;
  std::string                                             m_sJson;
  SJsonException                                           m_parseError;
  bool                                                    m_bValidationOk;

private:
  std::shared_ptr<SJsonInstructionNode>                   m_spNextNode;
};

//----------------------------------------------------------------------------------------
// CJSonSaxParser helper funcntions
//
std::variant<EArgumentType, std::false_type>
TypeFromNode(const std::shared_ptr<SJsonInstructionNode>& spNode,
             const std::string& sKey,
             bool& bArray)
{
  tInstructionMapType* argsDefinition = spNode->m_argsDefinition;
  SInstructionArgumentType* argDefinition = spNode->m_argDefinitionArr;

  EArgumentType ret = EArgumentType::eBool;
  if (nullptr != argsDefinition)
  {
    auto it = argsDefinition->find(QString::fromStdString(sKey));
    if (argsDefinition->end() == it) { it = argsDefinition->find("*"); } // wildcard to allow all commands as children
    if (argsDefinition->end() != it) { ret = it->second.m_type; bArray = false; }
  }
  else if (nullptr != argDefinition) { ret  = argDefinition->m_type; bArray = true; }
  else return std::false_type();

  return ret;
}

//----------------------------------------------------------------------------------------
//
void InsertValueIntoArgs(const QVariant& value,
                         tInstructionMapValue& actualArgs, const std::string& sKey,
                         EArgumentType type,
                         bool bArray)
{
  if (!bArray)
  {
    actualArgs.insert(
          { QString::fromStdString(sKey),
            ValuefromVariant(value, type) });
  }
  else
  {
    // vector not found -> insert new vector
    auto it = actualArgs.find(QString::fromStdString(sKey));
    if (actualArgs.end() == it)
    {
      actualArgs.insert({QString::fromStdString(sKey),
                         {type, tInstructionArrayValue()}});
    }

    it = actualArgs.find(QString::fromStdString(sKey));
    tInstructionArrayValue& array = std::get<tInstructionArrayValue>(it->second.m_value);
    array.push_back(ValuefromVariant(value, type));
  }
}

//----------------------------------------------------------------------------------------
// CJSonSaxParser implementation
//
bool CJSonSaxParser::null()
{
  // null values are not allowed
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::boolean(bool val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var(val);
        if (!var.convert(*foundValueType)) { return false; }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::number_integer(number_integer_t val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var(val);
        if (!var.convert(*foundValueType)) { return false; }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::number_unsigned(number_unsigned_t val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var(val);
        if (!var.convert(*foundValueType)) { return false; }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::number_float(number_float_t val, const string_t&)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var(val);
        if (!var.convert(*foundValueType)) { return false; }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::string(string_t& val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var(QString::fromStdString(val));
        if (!var.convert(*foundValueType)) { return false; }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::binary(binary_t& val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QByteArray arr;
        for (uchar c : val) { arr += c; }
        QVariant var(QString::fromUtf8(arr));
        if (!var.convert(*foundValueType)) { return false; }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::start_object(std::size_t elements)
{
  Q_UNUSED(elements)

  if (m_bParsingCommands)
  {
    // if we're in an ignored branch
    if (m_parseStack.top()->m_bIgnoreChildren)
    {
      CreateAndAddIgnoreNode();
      return true;
    }

    std::shared_ptr<IJsonInstructionBase> spCurrentCommand =
        m_parseStack.top()->m_wpCommand.lock();

    // no command is parsing yet
    if (nullptr == spCurrentCommand)
    {
      auto it = m_instructionMap.find(QString::fromStdString(m_sCurrentKey));
      if (m_instructionMap.end() != it)
      {
        std::shared_ptr<SJsonInstructionNode> spNode =
            std::make_shared<SJsonInstructionNode>();
        spNode->m_sName = QString::fromStdString(m_sCurrentKey);
        spNode->m_wpCommand = it->second;
        spNode->m_wpParent = m_parseStack.top();
        spNode->m_bIgnoreChildren = false;
        spNode->m_argsDefinition = &it->second->ArgList();
        m_parseStack.top()->m_spChildren.push_back(spNode);
        m_parseStack.push(spNode);
      }
      else
      {
        std::shared_ptr<SJsonInstructionNode> spNode =
            std::make_shared<SJsonInstructionNode>();
        spNode->m_sName = QString::fromStdString(m_sCurrentKey);
        spNode->m_bIgnoreChildren = false;
        spNode->m_wpParent = m_parseStack.top();
        m_parseStack.top()->m_spChildren.push_back(spNode);
        m_parseStack.push(spNode);
        return true;
      }
    }
    else
    {
      // a command is being parsed
      std::shared_ptr<SJsonInstructionNode> spParentNode = m_parseStack.top();
      tInstructionMapType* argsDefinition = spParentNode->m_argsDefinition;
      SInstructionArgumentType* argDefinitionArr = spParentNode->m_argDefinitionArr;
      if (nullptr != argsDefinition || nullptr != argDefinitionArr)
      {
        bool bArray = false;
        auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
        if (std::holds_alternative<EArgumentType>(type))
        {
          // is it a new object?
          if (EArgumentType::eObject == std::get<EArgumentType>(type)._to_integral())
          {
            if (!bArray)
            {
              auto itInstruction = m_instructionMap.find(QString::fromStdString(m_sCurrentKey));
              if (m_instructionMap.end() != itInstruction)
              {
                spParentNode->m_actualArgs.insert({QString::fromStdString(m_sCurrentKey),
                                                   {EArgumentType::eObject, QString::fromStdString(m_sCurrentKey)}});
                std::shared_ptr<SJsonInstructionNode> spNode = std::make_shared<SJsonInstructionNode>();
                spNode->m_sName = QString::fromStdString(m_sCurrentKey);
                spNode->m_wpCommand = itInstruction->second;
                spNode->m_wpParent = m_parseStack.top();
                spNode->m_bIgnoreChildren = false;
                spNode->m_argsDefinition = &itInstruction->second->ArgList();
                m_parseStack.top()->m_spChildren.push_back(spNode);
                m_parseStack.push(spNode);
              }
              else
              {
                CreateAndAddIgnoreNode();
              }
            }
            else
            {
              // vector not found -> insert new vector
              auto it = spParentNode->m_actualArgs.find(QString::fromStdString(m_sCurrentKey));
              if (spParentNode->m_actualArgs.end() == it)
              {
                spParentNode->m_actualArgs.insert({QString::fromStdString(m_sCurrentKey),
                                                  {std::get<EArgumentType>(type), tInstructionArrayValue()}});
              }

              it = spParentNode->m_actualArgs.find(QString::fromStdString(m_sCurrentKey));
              tInstructionArrayValue& array = std::get<tInstructionArrayValue>(it->second.m_value);
              const QString sElemName = QString::number(array.size());
              array.push_back({EArgumentType::eObject, sElemName});

              // insert mostly empty node
              std::shared_ptr<SJsonInstructionNode> spNode = std::make_shared<SJsonInstructionNode>();
              spNode->m_sName = sElemName;
              spNode->m_wpParent = m_parseStack.top();
              spNode->m_bIgnoreChildren = false;
              m_parseStack.top()->m_spChildren.push_back(spNode);
              m_parseStack.push(spNode);
            }
          }
          // is it part of the arguments
          else if (EArgumentType::eMap == std::get<EArgumentType>(type)._to_integral())
          {
            std::shared_ptr<SJsonInstructionNode> spNode = std::make_shared<SJsonInstructionNode>();
            spNode->m_sName = QString::fromStdString(m_sCurrentKey);
            spNode->m_wpCommand = spParentNode->m_wpCommand;
            spNode->m_wpParent = m_parseStack.top();
            spNode->m_bIgnoreChildren = false;
            if (!bArray)
            {
              auto itArg = argsDefinition->find(QString::fromStdString(m_sCurrentKey));
              if (argsDefinition->end() == itArg) { itArg = argsDefinition->find("*"); }
              spNode->m_argsDefinition = &std::get<tInstructionMapType>(itArg->second.m_nestedType);
            }
            else
            {
              // not valid json file requested in command
              return false;
            }
            m_parseStack.top()->m_spChildren.push_back(spNode);
            m_parseStack.push(spNode);
          }
          else
          {
            return false;
          }
        }
        else
        {
          CreateAndAddIgnoreNode();
        }
      }
      else
      {
        // we didn't find an argument definition
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::end_object()
{
  if (!m_parseStack.empty())
  {
    std::shared_ptr<SJsonInstructionNode> spChild = m_parseStack.top();
    m_parseStack.pop();
    if (m_parseStack.empty())
    {
      m_bParsingCommands = false;
    }
    else
    {
      m_sCurrentKey = m_parseStack.top()->m_sName.toStdString();

      std::shared_ptr<SJsonInstructionNode> spParent = m_parseStack.top();
      std::shared_ptr<IJsonInstructionBase> spChildCommand = spChild->m_wpCommand.lock();
      std::shared_ptr<IJsonInstructionBase> spParentCommand = spParent->m_wpCommand.lock();

      if (nullptr == spChildCommand)
      {
        // squash children
        auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
        spParent->m_spChildren.erase(itChild);
        spParent->m_spChildren.insert(spParent->m_spChildren.end(),
                                      spChild->m_spChildren.begin(), spChild->m_spChildren.end());
        for (auto& spChildChild : spChild->m_spChildren)
        {
          spChildChild->m_wpParent = spParent;
        }
      }
      else
      {
        if (nullptr != spParentCommand)
        {
          auto itArg = spParent->m_argsDefinition->find(spChild->m_sName);
          if (spParent->m_argsDefinition->end() != itArg)
          {
            // we popped a map type, so we need to insert the values into the arguments as a map value
            // we can also just remove the node from the children since we are done processing
            // the object
            if (EArgumentType::eMap == itArg->second.m_type._to_integral())
            {
              spParent->m_actualArgs.insert({spChild->m_sName,
                                             {EArgumentType::eMap, spChild->m_actualArgs}});
              auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
              spParent->m_spChildren.erase(itChild);
            }
          }
        }
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::start_array(std::size_t elements)
{
  Q_UNUSED(elements)
  if (m_bParsingCommands)
  {
    // if we're in an ignored branch
    if (m_parseStack.top()->m_bIgnoreChildren)
    {
      CreateAndAddIgnoreNode();
      return true;
    }

    std::shared_ptr<IJsonInstructionBase> spCurrentCommand =
        m_parseStack.top()->m_wpCommand.lock();

    // no command is parsing yet
    if (nullptr == spCurrentCommand)
    {
      std::shared_ptr<SJsonInstructionNode> spNode =
          std::make_shared<SJsonInstructionNode>();
      spNode->m_sName = QString::fromStdString(m_sCurrentKey);
      spNode->m_bIgnoreChildren = false;
      spNode->m_wpParent = m_parseStack.top();
      m_parseStack.top()->m_spChildren.push_back(spNode);
      m_parseStack.push(spNode);
      return true;
    }
    else
    {
      // a command is being parsed
      std::shared_ptr<SJsonInstructionNode> spParentNode = m_parseStack.top();
      tInstructionMapType* argsDefinition = spParentNode->m_argsDefinition;
      if (nullptr != argsDefinition)
      {
        auto itArg = argsDefinition->find(QString::fromStdString(m_sCurrentKey));
        if (argsDefinition->end() != itArg)
        {
          // is it an array?
          if (EArgumentType::eArray == itArg->second.m_type._to_integral())
          {
            std::shared_ptr<SJsonInstructionNode> spNode = std::make_shared<SJsonInstructionNode>();
            spNode->m_sName = QString::fromStdString(m_sCurrentKey);
            spNode->m_wpCommand = spParentNode->m_wpCommand;
            spNode->m_wpParent = m_parseStack.top();
            spNode->m_bIgnoreChildren = false;
            spNode->m_argDefinitionArr = std::get<std::shared_ptr<SInstructionArgumentType>>(itArg->second.m_nestedType).get();
            m_parseStack.top()->m_spChildren.push_back(spNode);
            m_parseStack.push(spNode);
          }
          else
          {
            return false;
          }
        }
        else
        {
          CreateAndAddIgnoreNode();
        }
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::end_array()
{
  if (m_bParsingCommands)
  {
    std::shared_ptr<SJsonInstructionNode> spChild = m_parseStack.top();
    m_parseStack.pop();
    if (m_parseStack.empty())
    {
      m_bParsingCommands = false;
    }
    else
    {
      m_sCurrentKey = m_parseStack.top()->m_sName.toStdString();

      std::shared_ptr<SJsonInstructionNode> spParent = m_parseStack.top();
      std::shared_ptr<IJsonInstructionBase> spChildCommand = spChild->m_wpCommand.lock();
      std::shared_ptr<IJsonInstructionBase> spParentCommand = spParent->m_wpCommand.lock();

      if (nullptr == spChildCommand)
      {
        // squash children
        auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
        spParent->m_spChildren.erase(itChild);
        spParent->m_spChildren.insert(spParent->m_spChildren.end(),
                                      spChild->m_spChildren.begin(), spChild->m_spChildren.end());
        for (auto& spChildChild : spChild->m_spChildren)
        {
          spChildChild->m_wpParent = spParent;
        }
      }
      else
      {
        if (nullptr != spParentCommand)
        {
          auto itArg = spParent->m_argsDefinition->find(spChild->m_sName);
          if (spParent->m_argsDefinition->end() != itArg)
          {
            // we popped an array type, so we need to insert the values into the arguments as an array value
            // we can also just remove the node from the children since we are done processing
            // the object
            if (EArgumentType::eArray == itArg->second.m_type._to_integral())
            {
              auto it = spChild->m_actualArgs.find(spChild->m_sName);
              if (spChild->m_actualArgs.end() != it)
              {
                spParent->m_actualArgs.insert({spChild->m_sName,
                                               {EArgumentType::eArray, it->second.m_value}});
                auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
                spParent->m_spChildren.erase(itChild);
              }
            }
          }
        }
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::key(string_t& val)
{
  // starting commands
  if (val == c_sCommandsNode)
  {
    m_bParsingCommands = true;

    std::shared_ptr<SJsonInstructionNode> spRotNode =
        std::make_shared<SJsonInstructionNode>();
    spRotNode->m_sName = QString::fromStdString(m_sCurrentKey);
    spRotNode->m_bIgnoreChildren = false;
    m_parseStack.push(spRotNode);
    m_pParent->m_vspBuiltCommands.push_back(
          {QString::fromStdString(m_sCurrentKey), spRotNode});
  }

  // store key
  m_sCurrentKey = val;
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::parse_error(std::size_t position, const std::string& last_token,
                                 const nlohmann::detail::exception& ex)
{
  Q_UNUSED(position)
  m_pParent->m_parseError = ExceptionToStruct(ex);
  m_pParent->m_parseError.m_sToken = QString::fromStdString(last_token);
  return true;
}

//----------------------------------------------------------------------------------------
//
void CJSonSaxParser::CreateAndAddIgnoreNode()
{
  // we don't add to parents children, so this object will be deleted once popped from stack
  std::shared_ptr<SJsonInstructionNode> spNode =
      std::make_shared<SJsonInstructionNode>();
  spNode->m_sName = QString::fromStdString(m_sCurrentKey);
  spNode->m_wpParent = m_parseStack.top();
  spNode->m_bIgnoreChildren = true;
  m_parseStack.push(spNode);
}

//----------------------------------------------------------------------------------------
// CJsonInstructionSetParserPrivate implementation
//
class CJsonInstructionSetParserPrivate
{
public:
  CJsonInstructionSetParserPrivate() :
    m_spParser(nullptr),
    m_jsonBaseSchema(),
    m_error()
  {

  }
  ~CJsonInstructionSetParserPrivate()
  {

  }

  //--------------------------------------------------------------------------------------
  //
  void ClearInstructions()
  {
    m_instructionMap.clear();
  }

  //--------------------------------------------------------------------------------------
  //
  SJsonException Error() const
  {
    return m_error;
  }

  //--------------------------------------------------------------------------------------
  //
  std::shared_ptr<CJsonInstructionSetRunner> ParseJson(const std::string& json)
  {
    auto spRunner =
        std::make_shared<CJsonInstructionSetRunner>();
    spRunner->m_pPrivate->m_jsonBaseSchema = m_jsonBaseSchema;
    if (spRunner->m_pPrivate->Validate(json))
    {
      spRunner->m_pPrivate->m_sJson = json;
      m_spParser.reset(new CJSonSaxParser(spRunner->m_pPrivate.get(), m_instructionMap));
      if (nlohmann::json::sax_parse(spRunner->m_pPrivate->m_sJson, m_spParser.get()))
      {
        m_error = SJsonException();
        return spRunner;
      }
      else
      {
        m_error = spRunner->m_pPrivate->ParseError();
        qWarning() << m_error.m_sException;
        return nullptr;
      }
    }
    else
    {
      m_error = spRunner->m_pPrivate->ParseError();
      qWarning() << m_error.m_sException;
      return nullptr;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void RegisterInstruction(const QString& sId,
                           const std::shared_ptr<IJsonInstructionBase>& spInstructionDefinition)
  {
    m_instructionMap.insert({sId, spInstructionDefinition});
  }

  //--------------------------------------------------------------------------------------
  //
  void RegisterInstructionSetPath(const QString& sId,
                                  const QStringList& vsInstructionSetPath)
  {
    m_instructionSetPath.insert({sId, vsInstructionSetPath});
  }

  //--------------------------------------------------------------------------------------
  //
  void SetJsonBaseSchema(const std::string& json)
  {
    try
    {
      m_jsonBaseSchema = nlohmann::json::parse(json);
    }
    catch (const std::exception &e)
    {
      m_error = ExceptionToStruct(e);
    }
  }

protected:
  std::unique_ptr<CJSonSaxParser>                          m_spParser;
  nlohmann::json                                           m_jsonBaseSchema;
  std::map<QString, QStringList>                           m_instructionSetPath;
  std::map<QString, std::shared_ptr<IJsonInstructionBase>> m_instructionMap;
  SJsonException                                           m_error;
};

//----------------------------------------------------------------------------------------
// CJsonInstructionSetRunner implementation
//
CJsonInstructionSetRunner::CJsonInstructionSetRunner() :
  m_pPrivate(std::make_unique<CJsonInstructionSetRunnerPrivate>())
{

}

CJsonInstructionSetRunner::~CJsonInstructionSetRunner()
{

}

//----------------------------------------------------------------------------------------
//
CJsonInstructionSetRunner::tRetVal
CJsonInstructionSetRunner::CallNextCommand(ERunerMode runMode)
{
  return m_pPrivate->CallNextCommand(runMode);
}

//----------------------------------------------------------------------------------------
//
CJsonInstructionSetRunner::tRetVal
CJsonInstructionSetRunner::Run(const QString& sInstructionSet, ERunerMode runMode)
{
  return m_pPrivate->Run(sInstructionSet, runMode);
}

//----------------------------------------------------------------------------------------
//
CJsonInstructionSetParser::CJsonInstructionSetParser(QObject* pParent) :
  QObject(pParent),
  m_spPtr(std::make_unique<CJsonInstructionSetParserPrivate>())
{

}

CJsonInstructionSetParser::~CJsonInstructionSetParser()
{

}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::ClearInstructions()
{
  m_spPtr->ClearInstructions();
}

//----------------------------------------------------------------------------------------
//
SJsonException CJsonInstructionSetParser::Error() const
{
  return m_spPtr->Error();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CJsonInstructionSetRunner>
CJsonInstructionSetParser::ParseJson(const QByteArray& json)
{
  return m_spPtr->ParseJson(QString::fromUtf8(json).toStdString());
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CJsonInstructionSetRunner>
CJsonInstructionSetParser::ParseJson(const QString& sJson)
{
  return m_spPtr->ParseJson(sJson.toStdString());
}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::RegisterInstruction(const QString& sId,
                                                    const std::shared_ptr<IJsonInstructionBase>& spInstructionDefinition)
{
  m_spPtr->RegisterInstruction(sId, spInstructionDefinition);
}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::RegisterInstructionSetPath(const QString& sId,
                                                           const QString& sInstructionSetPath)
{
  if (sInstructionSetPath == "/")
  {
    m_spPtr->RegisterInstructionSetPath(sId, QStringList() << QString());
  }
  else
  {
    QStringList list = sInstructionSetPath.split("/");
    m_spPtr->RegisterInstructionSetPath(sId, list);
  }
}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::SetJsonBaseSchema(const QByteArray& schema)
{
  m_spPtr->SetJsonBaseSchema(QString::fromUtf8(schema).toStdString());
}
