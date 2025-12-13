#ifndef SCRIPTTEXTBOX_H
#define SCRIPTTEXTBOX_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QColor>
#include <QJSValue>
#include <memory>

class CDatabaseManager;

namespace TextAlignment
{
  Q_NAMESPACE
  enum ETextAlignment
  {
    AlignLeft = Qt::AlignLeft,
    AlignRight = Qt::AlignRight,
    AlignCenter = Qt::AlignHCenter
  };
  Q_ENUM_NS(ETextAlignment)
}

//----------------------------------------------------------------------------------------
//
class CTextBoxSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CTextBoxSignalEmitter();
  ~CTextBoxSignalEmitter();

signals:
  void clearText();
  void getDialogue(QString sRequestId, QString sId, bool bIsRegexp, QStringList vsTags);
  void getDialogueReturnValue(QString sRequestId, QString sString, qint64 iTime, bool bWait,
                            QString sSoundResource, QStringList vsTags);
  void showButtonPrompts(QStringList vsLabels, QString sStoreIntoVar, QString sRequestId, bool bStoreIntoStorageInstead);
  void showButtonReturnValue(qint32 iIndex, QString sRequestId);
  void showInput(QString sDefault, QString sStoreIntoVar, QString sRequestId, bool bStoreIntoStorageInstead);
  void showInputReturnValue(QString sValue, QString sRequestId);
  void showText(QString sText, double dSkippableWaitS, QString sResource);
  void soundFinished(QString sResource);
  void stopResource(QString sResource);
  void textAlignmentChanged(qint32 alignment);
  void textBackgroundColorsChanged(std::vector<QColor> vColors);
  void textColorsChanged(std::vector<QColor> vColors);
  void textPortraitChanged(QString sResource);
  void waitSkipped();

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CTextBoxScriptCommunicator : public CScriptCommunicator
{
  public:
  CTextBoxScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CTextBoxScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptTextBox : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTextBox)

public:
  CScriptTextBox(std::weak_ptr<CScriptCommunicator> pCommunicator,
                 QPointer<QJSEngine> pEngine);
  CScriptTextBox(std::weak_ptr<CScriptCommunicator> pCommunicator,
                 QtLua::State* pState);
  ~CScriptTextBox();

public slots:
  void setBackgroundColors(QVariant color);
  void setTextAlignment(qint32 alignment);
  void setTextColors(QVariant color);
  void setTextPortrait(QVariant resource);
  qint32 showButtonPrompts(QVariant vsLabels);
  qint32 showButtonPrompts(QVariant vsLabels, QString sStoreInto);
  QString showInput();
  QString showInput(QString sDefault);
  QString showInput(QString sDefault, QString sStoreInto);
  QVariant getDialogue(QVariant data);
  void showDialogue(QVariant data);
  void showText(QString sText);
  void showText(QString sText, double dWaitTime);
  void showText(QString sText, double dWaitTime, bool bSkipable);
  void showText(QString sText, double dWaitTime, bool bSkipable, QVariant sResource);
  void clear();

signals:
  void SignalQuitLoop();
  void SignalPauseTimer();
  void SignalResumeTimer();

private:
  std::vector<QColor> GetColors(const QVariant& color, const QString& sSource);
  bool GetDialogueData(QVariant data, std::shared_ptr<QString>& sStringRet,
                       std::shared_ptr<qint32>& iTimeRet, std::shared_ptr<bool>& bWaitRet,
                       std::shared_ptr<QString>& sSoundResource,
                       std::shared_ptr<QStringList>& vsTagsRet);
  bool GetDialogueFromUi(QString sId, QString sString, bool bIsRegexp, QStringList vsTags,
                         std::shared_ptr<QString>& sStringRet, std::shared_ptr<qint32>& iTimeRet,
                         std::shared_ptr<bool>& bWaitRet, std::shared_ptr<QString>& sSoundResource,
                         std::shared_ptr<QStringList>& vsTagsRet);
  QString GetResourceName(const QVariant& resource, const QString& sMethod, bool* pbOk);

  std::shared_ptr<std::function<void()>> m_spStop;
  std::shared_ptr<std::function<void()>> m_spPause;
  std::shared_ptr<std::function<void()>> m_spResume;
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosChoice;
class CCommandEosPrompt;
class CCommandEosSay;
class CEosScriptTextBox : public CEosScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CEosScriptTextBox)

public:
  enum ESayMode
  {
    eAutoplay = 0,
    eInstant,
    ePause,
    eCustom
  };

  CEosScriptTextBox(std::weak_ptr<CScriptCommunicator> pCommunicator,
                    QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptTextBox();

  QString getTimerValue(const QString& sValue);
  void setTextAlignment(qint32 alignment);
  void setTextColors(const std::vector<QColor>& vColors);
  void setBackgroundColors(const std::vector<QColor>& vColors);
  qint32 showButtonPrompts(const QStringList& vsLabels);
  QString showInput(const QString& sStoreIntoVar);
  void showText(QString sText);
  void showText(QString sText, double dWaitTime, bool bSkipable);

signals:
  void SignalQuitLoop();
  void SignalPauseTimer();
  void SignalResumeTimer();

private:
  std::shared_ptr<CCommandEosChoice> m_spCommandChoice;
  std::shared_ptr<CCommandEosPrompt> m_spCommandPrompt;
  std::shared_ptr<CCommandEosSay>    m_spCommandSay;
  std::shared_ptr<std::function<void()>> m_spStop;
  std::shared_ptr<std::function<void()>> m_spPause;
  std::shared_ptr<std::function<void()>> m_spResume;
};

#endif // SCRIPTTEXTBOX_H
