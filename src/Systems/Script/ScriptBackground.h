#ifndef SCRIPTBACKGROUND_H
#define SCRIPTBACKGROUND_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QColor>
#include <memory>

class CDatabaseManager;
class CResourceScriptWrapper;

class CBackgroundSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT

public:
  CBackgroundSignalEmitter();
  ~CBackgroundSignalEmitter();

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;

signals:
  void backgroundColorChanged(QColor color);
  void backgroundTextureChanged(const QString& sResource);
};
Q_DECLARE_METATYPE(CBackgroundSignalEmitter)


//----------------------------------------------------------------------------------------
//
class CScriptBackground : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptBackground)

public:
  CScriptBackground(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                    QPointer<QJSEngine> pEngine);
  CScriptBackground(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                    QtLua::State* pState);
  ~CScriptBackground();

public slots:
  void setBackgroundColor(QVariant color);
  void setBackgroundTexture(QVariant resource);

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  QColor                           m_currentColor;
};

#endif // SCRIPTBACKGROUND_H
