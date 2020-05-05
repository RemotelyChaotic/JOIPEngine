#ifndef SCRIPTTEXTBOX_H
#define SCRIPTTEXTBOX_H

#include "ScriptObjectBase.h"
#include <QColor>
#include <QJSValue>
#include <memory>

class CDatabaseManager;

class CScriptTextBox : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTextBox)

public:
  CScriptTextBox(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
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
