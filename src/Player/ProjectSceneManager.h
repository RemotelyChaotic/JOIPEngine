#ifndef CPROJECTSCENEMANAGER_H
#define CPROJECTSCENEMANAGER_H

#include "ProjectEventTarget.h"
#include <memory>

class CProjectRunner;

class CProjectSceneManagerWrapper : public CProjectEventTargetWrapper
{
  Q_OBJECT
public:
  CProjectSceneManagerWrapper(QObject* pParent = nullptr);
  ~CProjectSceneManagerWrapper() override;

  void Initalize(std::weak_ptr<CProjectRunner> wpProjectRunner,
                 std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry);

public slots:
  void disable(QString sScene);
  void enable(QString sScene);
  bool isEnabled(QString sScene);
  QString getCurrentSceneId();
  void gotoScene(QString sScene);

private:
  std::weak_ptr<CProjectRunner>     m_wpProjectRunner;
};

#endif // CPROJECTSCENEMANAGER_H
