#ifndef WINDOWCONTEXT_H
#define WINDOWCONTEXT_H

#include "Enums.h"
#include <QObject>

class CWindowContext : public QObject
{
  Q_OBJECT
public:
  enum ETransitionDirection
  {
    eHorizontal,
    eVertical
  };

  explicit CWindowContext(QObject *parent = nullptr);
  ~CWindowContext();

signals:
  void SignalChangeAppOverlay(const QString& sImage);
  void SignalChangeAppState(EAppState newState, ETransitionDirection direction = eHorizontal);
  void SignalSetLeftButtonsVisible(bool bVisible);
  void SignalSetHelpButtonVisible(bool bVisible);
};

#endif // WINDOWCONTEXT_H
