#ifndef SCRIPTNOTIFICATION_H
#define SCRIPTNOTIFICATION_H

#include "CommonScriptHelpers.h"
#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QtCore/qmetaobject.h>
#include <QtLua/MetaType>
#include <memory>

class CDatabaseManager;
class CScriptRunnerSignalEmiter;

namespace IconAlignment
{
  Q_NAMESPACE
  enum EIconAlignment
  {
    AlignLeft = Qt::AlignLeft,
    AlignRight = Qt::AlignRight
  };
  Q_ENUM_NS(EIconAlignment)
}

//----------------------------------------------------------------------------------------
//
class CNotificationSignalEmiter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CNotificationSignalEmiter();
  ~CNotificationSignalEmiter();

signals:
  void clearNotifications();
  void hideNotification(QString sId);
  void iconAlignmentChanged(qint32 alignment);
  void portraitChanged(QString sResource);
  void shortcutChanged(QString sShortcut);
  void showNotification(QString sId, QString sTitle, QString sButtonText, double iTimeS, QString sOnButton, QString sOnTimeout);
  void showNotificationClick(QString sId, QString sOnButton);
  void showNotificationTimeout(QString sId, QString sOnTimeout);
  void textBackgroundColorChanged(QColor color);
  void textColorChanged(QColor color);
  void widgetBackgroundColorChanged(QColor color);
  void widgetColorChanged(QColor color);

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CNotificationScriptCommunicator : public CScriptCommunicator
{
  public:
  CNotificationScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CNotificationScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptNotification : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptNotification)

public:
  CScriptNotification(std::weak_ptr<CScriptCommunicator> pCommunicator,
                      QPointer<QJSEngine> pEngine);
  CScriptNotification(std::weak_ptr<CScriptCommunicator> pCommunicator,
                      QtLua::State* pState);
  ~CScriptNotification();

public slots:
  void clear();
  void hide(QString sId);
  void setIconAlignment(qint32 alignment);
  void setPortrait(QVariant resource);
  void show(QString sId, QString sTitle);
  void show(QString sId, QString sTitle, QVariant sButtonTextOrTimeout);
  void show(QString sId, QString sTitle, QVariant sButtonTextOrTimeout, QVariant onButtonOnTimeoutOrTime);
  void show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QVariant sOnButton);
  void show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QVariant sOnButton, QVariant sOnTimeout);
  void setShortcut(QString sShortcut);
  void setTextBackgroundColor(QVariant color);
  void setTextColor(QVariant color);
  void setWidgetBackgroundColor(QVariant color);
  void setWidgetColor(QVariant color);

  void registerClickCallback(const QString& sId, QVariant callback);
  void registerTimeouCallback(const QString& sId, QVariant callback);

signals:
  void SignalOverlayCleared();
  void SignalOverlayClosed(const QString& sId);
  void SignalOverlayRunAsync(const QString& sId, const QString& sScriptResource);

private slots:
  void HandleClicked(const QString& sId);
  void HandleTimeout(const QString& sId);

private:
  void Initialize();
  QColor GetColor(const QVariant& color, const QString& sSource);
  QString GetResource(const QVariant& resource, const QString& sSource);
  void HandleCallbacks(const QString& sId, std::map<QString, script::tCallbackValue>* pCallbacks);
  void Show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QString sOnButton, QString sOnTimeout);

  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  std::map<QString, script::tCallbackValue> m_clickCallbacks;
  std::map<QString, script::tCallbackValue> m_timeoutCallbacks;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosNotificationCreate;
class CCommandEosNotificationClose;
class CEosScriptNotification : public CEosScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CEosScriptNotification)

public:
  CEosScriptNotification(std::weak_ptr<CScriptCommunicator> pCommunicator,
                         QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptNotification();

  QString GetTimerValue(const QString& sValue);
  void Hide(QString sId);
  void Show(QString sId, const QString& sTitle, const QString&  sButtonText,
            double dTimeS, const QString&  sOnButton, const QString&  sOnTimeout);

signals:
  void SignalOverlayClosed(const QString& sId);
  void SignalOverlayRunAsync(const QString& sId);

private:
  std::shared_ptr<CCommandEosNotificationCreate> m_spCommandCreate;
  std::shared_ptr<CCommandEosNotificationClose>  m_spCommandClose;
};

#endif // SCRIPTNOTIFICATION_H
