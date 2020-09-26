#ifndef CJSONINSTRUCTIONSETRUNNER_H
#define CJSONINSTRUCTIONSETRUNNER_H

#include <QString>
#include <memory>

class CJsonInstructionSetRunnerPrivate;

class CJsonInstructionSetRunner
{
  friend class CJsonInstructionSetParserPrivate;

public:
  explicit CJsonInstructionSetRunner();
  ~CJsonInstructionSetRunner();

  bool CallNextCommand();
  bool Run(const QString& sInstructionSet);

protected:
  std::unique_ptr<CJsonInstructionSetRunnerPrivate> m_pPrivate;
};

#endif // CJSONINSTRUCTIONSETRUNNER_H
