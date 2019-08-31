#ifndef SCRIPTICON_H
#define SCRIPTICON_H

#include <QJSEngine>
#include <QJSValue>

class CScriptIcon : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptIcon)

public:
  CScriptIcon(QJSEngine* pEngine);
  ~CScriptIcon();

public slots:
  void hide();
  void show(QJSValue resource);

private:
  QJSEngine*               m_pEngine;
};

#endif // SCRIPTICON_H
