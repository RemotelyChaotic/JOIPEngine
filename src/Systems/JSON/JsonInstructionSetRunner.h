#ifndef CJSONINSTRUCTIONSETRUNNER_H
#define CJSONINSTRUCTIONSETRUNNER_H

#include <QObject>

class CJsonInstructionSetRunner : public QObject
{
  Q_OBJECT
public:
  explicit CJsonInstructionSetRunner(QObject* pParent = nullptr);
  ~CJsonInstructionSetRunner();
};

#endif // CJSONINSTRUCTIONSETRUNNER_H
