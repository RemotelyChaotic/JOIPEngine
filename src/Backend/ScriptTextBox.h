#ifndef SCRIPTTEXTBOX_H
#define SCRIPTTEXTBOX_H

#include "Resource.h"
#include <QColor>
#include <QJSEngine>
#include <QJSValue>

class CScriptTextBox : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTextBox)

  Q_PROPERTY(QJSValue        backgroundColor   READ getBackgroundColor   WRITE setBackgroundColor)
  Q_PROPERTY(QJSValue        textColor         READ getTextColor         WRITE setTextColor)

public:
  CScriptTextBox(QJSEngine* pEngine);
  ~CScriptTextBox();

  QJSValue getBackgroundColor();
  void setBackgroundColor(QJSValue color);

  QJSValue getTextColor();
  void setTextColor(QJSValue color);

public slots:
  void showButtonPrompts(QJSValue vsLabels);
  void showText(const QString& sText);
  void clear();

private:
  QJSEngine*               m_pEngine;
};

#endif // SCRIPTTEXTBOX_H
