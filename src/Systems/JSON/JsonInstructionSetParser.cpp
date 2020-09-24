#include "JsonInstructionSetParser.h"
#include "JsonInstructionSetRunner.h"
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
    m_spValidationErrorHandler(std::make_unique<CustomErrorHandler>()),
    m_validator(nullptr, InstructionSetFormatChecker, nullptr),
    m_jsonBaseSchema(),
    m_json()
  {

  }
  ~CJsonInstructionSetRunnerPrivate()
  {

  }

  //--------------------------------------------------------------------------------------
  //
  bool Run()
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
      return false;
    }

    // validate the document - uses a custom error-handler, because we don't like
    // to throw around things
    m_validator.validate(m_json, *m_spValidationErrorHandler);
    if (static_cast<bool>(*m_spValidationErrorHandler))
    {
      return false;
    }

    // validation success, start actually running the commands

    return true;
  }

protected:
  std::unique_ptr<CustomErrorHandler>   m_spValidationErrorHandler;
  nlohmann::json_schema::json_validator m_validator;
  nlohmann::json m_jsonBaseSchema;
  nlohmann::json m_json;
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
    auto spRunner = std::make_shared<CJsonInstructionSetRunner>();
    spRunner->m_pPrivate->m_jsonBaseSchema = m_jsonBaseSchema;
    spRunner->m_pPrivate->m_json = nlohmann::json::parse(json);
    return spRunner;
  }

  //--------------------------------------------------------------------------------------
  //
  void SetJsonBaseSchema(const std::string& json)
  {
    m_jsonBaseSchema = nlohmann::json::parse(json);
  }

protected:
  nlohmann::json m_jsonBaseSchema;
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
bool CJsonInstructionSetRunner::Run()
{
  return m_pPrivate->Run();
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
void CJsonInstructionSetParser::RegisterInstructionSchema(const QString& sId,
                                                          const JsonInstructionBase& instructionDefinition)
{

};

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::SetJsonBaseSchema(const QByteArray& schema)
{
  m_spPtr->SetJsonBaseSchema(QString::fromUtf8(schema).toStdString());
}
