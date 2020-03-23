#ifndef WINDOWCONTEXT_H
#define WINDOWCONTEXT_H

#include "Enums.h"
#include <QObject>

class CWindowContext : public QObject
{
  Q_OBJECT
public:
  explicit CWindowContext(QObject *parent = nullptr);
  ~CWindowContext();

signals:
  void SignalChangeAppState(EAppState newState);
  void SignalSetHelpButtonVisible(bool bVisible);
};

#endif // WINDOWCONTEXT_H
