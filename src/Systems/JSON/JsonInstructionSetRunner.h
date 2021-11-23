#ifndef CJSONINSTRUCTIONSETRUNNER_H
#define CJSONINSTRUCTIONSETRUNNER_H

#include "JsonInstructionTypes.h"
#include <QVariant>
#include <any>

class CJsonInstructionSetRunnerPrivate;

struct SRunnerRetVal
{
  bool     m_bHasMoreCommands = false;
  std::any m_retVal = std::any();
};

//----------------------------------------------------------------------------------------
//
class CJsonInstructionSetRunner : public QObject
{
  Q_OBJECT
  friend class CJsonInstructionSetParserPrivate;

public:
  typedef std::variant<SJsonException, SRunnerRetVal /*Has more commands*/> tRetVal;

  explicit CJsonInstructionSetRunner();
  ~CJsonInstructionSetRunner();

  tRetVal CallNextCommand(ERunerMode runMode = ERunerMode::eRunOne, bool bBlocking = true);
  void Interrupt();
  tRetVal Run(const QString& sInstructionSet, ERunerMode runMode = ERunerMode::eRunOne, bool bBlocking = true);

signals:
  void CommandRetVal(CJsonInstructionSetRunner::tRetVal retVal);

protected:
  std::unique_ptr<CJsonInstructionSetRunnerPrivate> m_pPrivate;
};

Q_DECLARE_METATYPE(CJsonInstructionSetRunner::tRetVal)

#endif // CJSONINSTRUCTIONSETRUNNER_H
