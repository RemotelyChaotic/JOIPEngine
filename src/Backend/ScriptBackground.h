#ifndef SCRIPTBACKGROUND_H
#define SCRIPTBACKGROUND_H

#include "Resource.h"
#include <QColor>
#include <QJSEngine>

class CScriptBackground : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptBackground)

  Q_PROPERTY(QJSValue        backgroundColor   READ getBackgroundColor   WRITE setBackgroundColor)
  Q_PROPERTY(QJSValue        backgroundTexture READ getBackgroundTexture WRITE setBackgroundTexture)

public:
  CScriptBackground(QJSEngine* pEngine);
  ~CScriptBackground();

  QJSValue getBackgroundColor();
  void setBackgroundColor(QJSValue color);

  QJSValue getBackgroundTexture();
  void setBackgroundTexture(QJSValue resource);

private:
  QJSEngine*               m_pEngine;
};

#endif // SCRIPTBACKGROUND_H
