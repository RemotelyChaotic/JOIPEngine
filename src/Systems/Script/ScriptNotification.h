#ifndef SCRIPTNOTIFICATION_H
#define SCRIPTNOTIFICATION_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QJSValue>
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

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;

signals:
  void clearNotifications();
  void hideNotification(QString sId);
  void iconAlignmentChanged(qint32 alignment);
  void portraitChanged(QString sResource);
  void showNotification(QString sId, QString sTitle, QString sButtonText, double iTimeS, QString sOnButton, QString sOnTimeout);
  void showNotificationClick(QString sId, QString sOnButton);
  void showNotificationTimeout(QString sId, QString sOnTimeout);
  void textBackgroundColorChanged(QColor color);
  void textColorChanged(QColor color);
  void widgetBackgroundColorChanged(QColor color);
  void widgetColorChanged(QColor color);
};
Q_DECLARE_METATYPE(CNotificationSignalEmiter)

//----------------------------------------------------------------------------------------
//
class CScriptNotification : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptNotification)

public:
  CScriptNotification(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                      QPointer<QJSEngine> pEngine);
  ~CScriptNotification();

public slots:
  void clear();
  void hide(QString sId);
  void setIconAlignment(qint32 alignment);
  void setPortrait(QJSValue resource);
  void show(QString sId, QString sTitle);
  void show(QString sId, QString sTitle, QJSValue sButtonTextOrTimeout);
  void show(QString sId, QString sTitle, QJSValue sButtonTextOrTimeout, QJSValue onButtonOnTimeoutOrTime);
  void show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QJSValue sOnButton);
  void show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QJSValue sOnButton, QJSValue sOnTimeout);
  void setTextBackgroundColor(QJSValue color);
  void setTextColor(QJSValue color);
  void setWidgetBackgroundColor(QJSValue color);
  void setWidgetColor(QJSValue color);

signals:
  void SignalOverlayCleared();
  void SignalOverlayClosed(const QString& sId);
  void SignalOverlayRunAsync(const QString& sId, const QString& sScriptResource);

private:
  QColor GetColor(const QJSValue& color, const QString& sSource);
  QString GetResource(const QJSValue& resource, const QString& sSource);
  void Show(QString sId, QString sTitle, QString sButtonText, double dTimeS, QString sOnButton, QString sOnTimeout);

  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
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
  CEosScriptNotification(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                         QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptNotification();

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
