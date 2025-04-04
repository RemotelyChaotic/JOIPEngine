#ifndef CJSONINSTRUCTIONSETRUNNER_H
#define CJSONINSTRUCTIONSETRUNNER_H

#include "JsonInstructionTypes.h"
#include <QVariant>
#include <any>

class IJsonInstructionBase;
class CJsonInstructionNode;
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
  friend class CJsonInstructionSetRunnerPrivate;

public:
  typedef std::variant<SJsonException, SRunnerRetVal /*Has more commands*/> tRetVal;

  explicit CJsonInstructionSetRunner();
  ~CJsonInstructionSetRunner();

  tRetVal CallNextCommand(ERunerMode runMode = ERunerMode::eRunOne, bool bBlocking = true);
  std::shared_ptr<IJsonInstructionBase> Instruction(const QString& sName) const;
  void Interrupt();
  bool IsRunning() const;
  tRetVal Run(const QString& sInstructionSet, ERunerMode runMode = ERunerMode::eRunOne, bool bBlocking = true);

  const std::vector<std::pair<QString, std::shared_ptr<CJsonInstructionNode>>>& Nodes() const;

signals:
  void CommandRetVal(CJsonInstructionSetRunner::tRetVal retVal);
  void Fork(std::shared_ptr<CJsonInstructionSetRunner> spNewRunner, const QString& sForkCommandsName, bool bAutorun);

protected:
  std::unique_ptr<CJsonInstructionSetRunnerPrivate> m_pPrivate;
};

Q_DECLARE_METATYPE(CJsonInstructionSetRunner::tRetVal)
Q_DECLARE_METATYPE(std::shared_ptr<CJsonInstructionSetRunner>)

#endif // CJSONINSTRUCTIONSETRUNNER_H
