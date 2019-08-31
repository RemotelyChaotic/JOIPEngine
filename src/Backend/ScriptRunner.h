#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include "ThreadedSystem.h"
#include <QJSEngine>
#include <memory>

class CSettings;

class CScriptRunner : public CThreadedObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptRunner)

public:
  CScriptRunner();
  ~CScriptRunner() override;

public slots:
  void Initialize() override;
  void Deinitialize() override;

private:
  std::shared_ptr<CSettings>             m_spSettings;
  std::unique_ptr<QJSEngine>             m_spScriptEngine;
};

#endif // SCRIPTRUNNER_H
