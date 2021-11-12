#ifndef CJSONINSTRUCTIONSETRUNNER_H
#define CJSONINSTRUCTIONSETRUNNER_H

#include "JsonInstructionTypes.h"
#include <QVariant>

class CJsonInstructionSetRunnerPrivate;

//----------------------------------------------------------------------------------------
//
class CJsonInstructionSetRunner : public QObject
{
  Q_OBJECT
  friend class CJsonInstructionSetParserPrivate;

public:
  typedef std::variant<SJsonException, bool /*Has more commands*/> tRetVal;

  explicit CJsonInstructionSetRunner();
  ~CJsonInstructionSetRunner();

  tRetVal CallNextCommand(ERunerMode runMode = ERunerMode::eRunOne, bool bBlocking = true);
  tRetVal Run(const QString& sInstructionSet, ERunerMode runMode = ERunerMode::eRunOne, bool bBlocking = true);

signals:
  void CommandRetVal(CJsonInstructionSetRunner::tRetVal retVal);

protected:
  std::unique_ptr<CJsonInstructionSetRunnerPrivate> m_pPrivate;
};

Q_DECLARE_METATYPE(CJsonInstructionSetRunner::tRetVal)

#endif // CJSONINSTRUCTIONSETRUNNER_H
