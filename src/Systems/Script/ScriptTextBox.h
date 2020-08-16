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

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);

signals:
  void clearText();
  void showButtonPrompts(QStringList vsLabels);
  void showButtonReturnValue(qint32 iIndex);
  void showInput();
  void showInputReturnValue(QString sValue);
  void showText(QString sText);
  void textAlignmentChanged(qint32 alignment);
  void textBackgroundColorsChanged(std::vector<QColor> vColors);
  void textColorsChanged(std::vector<QColor> vColors);
  void textPortraitChanged(QString sResource);
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
  void setTextAlignment(qint32 alignment);
  void setTextColors(QJSValue color);
  void setTextPortrait(QJSValue resource);
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
