#ifndef CPROJECTSCENEMANAGER_H
#define CPROJECTSCENEMANAGER_H

#include "ProjectEventTarget.h"
#include <memory>

class CSceneNodeResolver;

class CProjectSceneManagerWrapper : public CProjectEventTargetWrapper
{
  Q_OBJECT
public:
  CProjectSceneManagerWrapper(QObject* pParent = nullptr);
  ~CProjectSceneManagerWrapper() override;

  void Dispatched(const QString& sEvent) override;
  QString EventTarget() override;
  void Initalize(std::weak_ptr<CSceneNodeResolver> wpSceneNodeResolver,
                 std::weak_ptr<CProjectEventCallbackRegistry> wpRegistry);

public slots:
  bool contains(QString sScene);
  void disable(QString sScene);
  void enable(QString sScene);
  bool isEnabled(QString sScene);
  QString getCurrentSceneId();
  void gotoScene(QString sScene);

private:
  std::weak_ptr<CSceneNodeResolver>     m_wpSceneNodeResolver;
};

#endif // CPROJECTSCENEMANAGER_H
