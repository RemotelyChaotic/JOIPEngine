#ifndef SCRIPTTEXTBOX_H
#define SCRIPTTEXTBOX_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QColor>
#include <QJSValue>
#include <memory>

class CDatabaseManager;


class CTextBoxSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CTextBoxSignalEmitter();
  ~CTextBoxSignalEmitter();

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);

signals:
  void clearText();
  void showButtonPrompts(QStringList vsLabels);
  void showButtonReturnValue(qint32 iIndex);
  void showInput();
  void showInputReturnValue(QString sValue);
  void showText(QString sText);
  void textBackgroundColorsChanged(std::vector<QColor> vColors);
  void textColorsChanged(std::vector<QColor> vColors);
};
Q_DECLARE_METATYPE(CTextBoxSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptTextBox : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTextBox)

public:
  CScriptTextBox(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                 QPointer<QJSEngine> pEngine);
  ~CScriptTextBox();

public slots:
  void setBackgroundColors(QJSValue color);
  void setTextColors(QJSValue color);
  qint32 showButtonPrompts(QJSValue vsLabels);
  QString showInput();
  void showText(QString sText);
  void clear();

signals:
  void SignalQuitLoop();

private:
  std::vector<QColor> GetColors(const QJSValue& color, const QString& sSource);

  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // SCRIPTTEXTBOX_H
