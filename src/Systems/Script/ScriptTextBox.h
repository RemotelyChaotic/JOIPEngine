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

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;
  std::shared_ptr<CScriptObjectBase> CreateNewSequenceObject() override;

signals:
  void clearText();
  void showButtonPrompts(QStringList vsLabels, QString sRequestId);
  void showButtonReturnValue(qint32 iIndex, QString sRequestId);
  void showInput(QString sStoreIntoVar, QString sRequestId, bool bStoreIntoStorageInstead);
  void showInputReturnValue(QString sValue, QString sRequestId);
  void showText(QString sText, double dSkippableWaitS);
  void textAlignmentChanged(qint32 alignment);
  void textBackgroundColorsChanged(std::vector<QColor> vColors);
  void textColorsChanged(std::vector<QColor> vColors);
  void textPortraitChanged(QString sResource);
  void waitSkipped();
};
Q_DECLARE_METATYPE(CTextBoxSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptTextBox : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTextBox)

public:
  CScriptTextBox(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                 QPointer<QJSEngine> pEngine);
  CScriptTextBox(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                 QtLua::State* pState);
  ~CScriptTextBox();

public slots:
  void setBackgroundColors(QVariant color);
  void setTextAlignment(qint32 alignment);
  void setTextColors(QVariant color);
  void setTextPortrait(QVariant resource);
  qint32 showButtonPrompts(QVariant vsLabels);
  QString showInput();
  void showText(QString sText);
  void showText(QString sText, double dWaitTime);
  void showText(QString sText, double dWaitTime, bool bSkipable);
  void clear();

signals:
  void SignalQuitLoop();

private:
  std::vector<QColor> GetColors(const QVariant& color, const QString& sSource);

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

  CEosScriptTextBox(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                    QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptTextBox();

  void setTextAlignment(qint32 alignment);
  void setTextColors(const std::vector<QColor>& vColors);
  void setBackgroundColors(const std::vector<QColor>& vColors);
  qint32 showButtonPrompts(const QStringList& vsLabels);
  QString showInput(const QString& sStoreIntoVar);
  void showText(QString sText);
  void showText(QString sText, double dWaitTime, bool bSkipable);

signals:
  void SignalQuitLoop();

private:
  std::shared_ptr<CCommandEosChoice> m_spCommandChoice;
  std::shared_ptr<CCommandEosPrompt> m_spCommandPrompt;
  std::shared_ptr<CCommandEosSay>    m_spCommandSay;
};

#endif // SCRIPTTEXTBOX_H
