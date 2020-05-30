#ifndef SCRIPTBACKGROUND_H
#define SCRIPTBACKGROUND_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QColor>
#include <memory>

class CDatabaseManager;
class CResource;

class CBackgroundSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT

public:
  CBackgroundSignalEmitter();
  ~CBackgroundSignalEmitter();

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);

signals:
  void backgroundColorChanged(QColor color);
  void backgroundTextureChanged(const QString& sResource);
};
Q_DECLARE_METATYPE(CBackgroundSignalEmitter)


//----------------------------------------------------------------------------------------
//
class CScriptBackground : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptBackground)

public:
  CScriptBackground(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                    QPointer<QJSEngine> pEngine);
  ~CScriptBackground();

public slots:
  void setBackgroundColor(QJSValue color);
  void setBackgroundTexture(QJSValue resource);

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  QColor                           m_currentColor;
};

#endif // SCRIPTBACKGROUND_H
