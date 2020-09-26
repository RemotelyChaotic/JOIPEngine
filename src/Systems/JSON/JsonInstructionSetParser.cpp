#include "JsonInstructionSetParser.h"
#include "JsonInstructionSetRunner.h"
#include "JsonInstructionNode.h"
#include <nlohmann/json-schema.hpp>
#include <QDebug>
#include <string>

namespace  {
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
      void error(const nlohmann::json_pointer<nlohmann::basic_json<>> &pointer,
                 const nlohmann::json &instance,
                 const std::string &message) override
      {
          nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
          qWarning() << "Validation of schema failed:" << QString::fromStdString(pointer.to_string())
                     << ":" << QString::fromStdString(message);
      }
  };
}

//----------------------------------------------------------------------------------------
//
class CJsonInstructionSetRunnerPrivate
{
  friend class CJsonInstructionSetParserPrivate;

public:
  CJsonInstructionSetRunnerPrivate() :
    m_vspBuiltCommands(),
    m_spValidationErrorHandler(std::make_unique<CustomErrorHandler>()),
    m_validator(nullptr, InstructionSetFormatChecker, nullptr),
    m_jsonBaseSchema(),
    m_json(),
    m_bValidationOk(false),
    m_spNextNode(nullptr)
  {
  }

  ~CJsonInstructionSetRunnerPrivate()
  {
  }

  //--------------------------------------------------------------------------------------
  //
  bool Validate()
  {
    // set schema
    try
    {
      m_validator.set_root_schema(m_jsonBaseSchema);
    }
    catch (const std::exception &e)
    {
      qWarning() << QString(QT_TR_NOOP("Validation of schema failed: %1"))
                    .arg(e.what());
      m_bValidationOk = false;
    }

    // validate the document - uses a custom error-handler, because we don't like
    // to throw around things
    m_validator.validate(m_json, *m_spValidationErrorHandler);
    if (static_cast<bool>(*m_spValidationErrorHandler))
    {
      m_bValidationOk = false;
    }

    m_bValidationOk = true;

    return m_bValidationOk;
  }

  //--------------------------------------------------------------------------------------
  //
  bool Build(const std::map<QString, QStringList>& instructionSetPath,
             const std::map<QString, std::shared_ptr<IJsonInstructionBase>>& instructionMap)
  {
    if (!m_bValidationOk) { return false; }

    for (auto it = instructionSetPath.begin(); instructionSetPath.end() != it; ++it)
    {
      // find path in json
      nlohmann::json foundElem = m_json;
      for (const QString& sPathElem : it->second)
      {
        if (sPathElem.isEmpty()) { break; }
        if (m_json.contains(sPathElem.toStdString()))
        {
          if (m_json[sPathElem.toStdString()].is_object())
          {
            foundElem = m_json[sPathElem.toStdString()];
          }
        }
      }

      // get all children that have a commands array
      for (auto jsonIt = foundElem.begin(); foundElem.end() != jsonIt; ++jsonIt)
      {
        if (jsonIt.value().contains("commands"))
        {
          nlohmann::json commands = jsonIt.value()["commands"];
          if (commands.is_array())
          {
            std::vector<std::shared_ptr<SJsonInstructionNode>> vspNewCommands;
            std::shared_ptr<SJsonInstructionNode> spRootNode =
                std::make_shared<SJsonInstructionNode>();

            // iterate over commands
            for (auto commandIt = commands.begin(); commands.end() != commandIt; ++commandIt)
            {
              // check if known commands contains command
              for (auto instructionIt = instructionMap.begin(); instructionMap.end() != instructionIt; ++instructionIt)
              {
                if (commandIt.value().contains(instructionIt->first.toStdString()) &&
                    nullptr != instructionIt->second)
                {
                  nlohmann::json command = commandIt.value()[instructionIt->first.toStdString()];
                  std::shared_ptr<IJsonInstructionBase> spInstructionBase = instructionIt->second;
                  const std::map<QString, QVariant::Type>& args = spInstructionBase->ArgList();

                  // create command node
                  std::shared_ptr<SJsonInstructionNode> spNode =
                      std::make_shared<SJsonInstructionNode>();
                  spNode->m_wpCommand = spInstructionBase;
                  spNode->m_wpParent = spRootNode;

                  // get arguments
                  for (const auto& argPair : args)
                  {
                    std::string argName = argPair.first.toStdString();
                    auto itFound = std::find_if(command.begin(), command.end(),
                        [&argName](const nlohmann::json& val){
                      return val.contains(argName);
                    });
                    if (command.end() != itFound)
                    {
                      QVariant varArg = GetVariant(itFound.value()[argName]);
                      if (varArg.canConvert(argPair.second) && varArg.isValid())
                      {
                        varArg.convert(argPair.second);
                        spNode->m_actualArgs.insert(argPair.first, varArg);
                      }
                    }
                  }

                  vspNewCommands.push_back(spNode);
                  break;
                }
              }
            }
            spRootNode->m_spChildren = vspNewCommands;
            m_vspBuiltCommands.push_back({QString::fromStdString(jsonIt.key()), spRootNode});
          }
        }
      }
    }

    return true;
  }

  //--------------------------------------------------------------------------------------
  //
  bool CallNextCommand()
  {
    if (nullptr != m_spNextNode)
    {
      bool bOk = CallCommand();
      if (!bOk) { return false; }
      return NextCommand();
    }
    return false;
  }

  //--------------------------------------------------------------------------------------
  //
  bool Run(const QString& sInstructionSet)
  {
    if (!m_bValidationOk) { return false; }

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
        return CallNextCommand();
      }
    }

    return false;
  }


protected:
  //--------------------------------------------------------------------------------------
  //
  bool CallCommand()
  {
    if (nullptr != m_spNextNode)
    {
      if (auto spCommand = m_spNextNode->m_wpCommand.lock())
      {
        spCommand->Call(m_spNextNode->m_actualArgs);
        return true;
      }
    }
    return false;
  }

  //--------------------------------------------------------------------------------------
  //
  bool NextCommand()
  {
    if (nullptr != m_spNextNode)
    {
      // dfs
      if (m_spNextNode->m_spChildren.size() > 0)
      {
        m_spNextNode = *m_spNextNode->m_spChildren.begin();
        return true;
      }
      else
      {
        if (auto spParent = m_spNextNode->m_wpParent.lock())
        {
          auto it =
            std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), m_spNextNode);
          qint32 iIndex = std::distance(spParent->m_spChildren.begin(), it);
          if (static_cast<qint32>(spParent->m_spChildren.size()) > iIndex+1)
          {
            m_spNextNode = spParent->m_spChildren[static_cast<size_t>(iIndex+1)];
            return true;
          }
        }
      }
    }
    // nothing found -> reset since all commands have run
    m_spNextNode = nullptr;
    return false;
  }

  //--------------------------------------------------------------------------------------
  //
  QVariant GetVariant(nlohmann::json& json)
  {
    QVariant varArg;
    switch(json.type())
    {
      case nlohmann::json::value_t::boolean:
      {
        bool bVal = json.get<bool>();
        varArg = bVal;
      } break;
      case nlohmann::json::value_t::string:
      {
        std::string sVal = json.get<std::string>();
        varArg = QString::fromStdString(sVal);
      } break;
      case nlohmann::json::value_t::number_integer:
      {
        long long iVal = json.get<int>();
        varArg = iVal;
      } break;
      case nlohmann::json::value_t::number_unsigned:
      {
        unsigned long long uiVal = json.get<unsigned>();
        varArg = uiVal;
      } break;
      case nlohmann::json::value_t::number_float:
      {
        double dVal = json.get<double>();
        varArg = dVal;
      } break;
      case nlohmann::json::value_t::object:
      {
        QVariantMap map;
        for (auto it = json.begin(); json.end() != it; ++it)
        {
          map.insert(QString::fromStdString(it.key()), GetVariant(it.value()));
        }
        varArg = map;
      }
      case nlohmann::json::value_t::array:
      {
        QVariantList list;
        for (auto it = json.begin(); json.end() != it; ++it)
        {
          list.push_back(GetVariant(it.value()));
        }
        varArg = list;
      }
      case nlohmann::json::value_t::binary: // fallthrough
      case nlohmann::json::value_t::null: // fallthrough
      case nlohmann::json::value_t::discarded:  // fallthrough
      default: break;
    }
    return varArg;
  }

  std::vector<std::pair<QString, std::shared_ptr<SJsonInstructionNode>>>
                                                          m_vspBuiltCommands;
  std::unique_ptr<CustomErrorHandler>                     m_spValidationErrorHandler;
  nlohmann::json_schema::json_validator                   m_validator;
  nlohmann::json                                          m_jsonBaseSchema;
  nlohmann::json                                          m_json;
  bool                                                    m_bValidationOk;

private:
  std::shared_ptr<SJsonInstructionNode>                   m_spNextNode;
};

//----------------------------------------------------------------------------------------
//
class CJsonInstructionSetParserPrivate
{
public:
  CJsonInstructionSetParserPrivate() :
    m_jsonBaseSchema()
  {

  }
  ~CJsonInstructionSetParserPrivate()
  {

  }

  //--------------------------------------------------------------------------------------
  //
  std::shared_ptr<CJsonInstructionSetRunner> ParseJson(const std::string& json)
  {
    auto spRunner =
        std::make_shared<CJsonInstructionSetRunner>();
    spRunner->m_pPrivate->m_jsonBaseSchema = m_jsonBaseSchema;
    spRunner->m_pPrivate->m_json = nlohmann::json::parse(json);
    if (spRunner->m_pPrivate->Validate())
    {
      if (spRunner->m_pPrivate->Build(m_instructionSetPath, m_instructionMap))
      {
        return spRunner;
      }
      else
      {
        return nullptr;
      }
    }
    else
    {
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
    m_jsonBaseSchema = nlohmann::json::parse(json);
  }

protected:
  nlohmann::json                                           m_jsonBaseSchema;
  std::map<QString, QStringList>                           m_instructionSetPath;
  std::map<QString, std::shared_ptr<IJsonInstructionBase>> m_instructionMap;
};

//----------------------------------------------------------------------------------------
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
bool CJsonInstructionSetRunner::CallNextCommand()
{
  return m_pPrivate->CallNextCommand();
}

//----------------------------------------------------------------------------------------
//
bool CJsonInstructionSetRunner::Run(const QString& sInstructionSet)
{
  return m_pPrivate->Run(sInstructionSet);
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
std::shared_ptr<CJsonInstructionSetRunner>
CJsonInstructionSetParser::ParseJson(const QByteArray& json)
{
  return m_spPtr->ParseJson(QString::fromUtf8(json).toStdString());
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
