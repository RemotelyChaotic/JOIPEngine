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
  void SignalChangeAppOverlay(const QString& sImage);
  void SignalChangeAppState(EAppState newState);
  void SignalSetDownloadButtonVisible(bool bVisible);
  void SignalSetHelpButtonVisible(bool bVisible);
};

#endif // WINDOWCONTEXT_H
