#ifndef OVERLAYMANAGER_H
#define OVERLAYMANAGER_H

#include "ThreadedSystem.h"
#include <QObject>
#include <vector>

class COverlayBase;

class COverlayManager : public CSystemBase
{
  Q_OBJECT

public:
  COverlayManager();
  ~COverlayManager() override;

public slots:
  void Initialize() override;
  void Deinitialize() override;

  void RebuildOverlayOrder();
  void RegisterOverlay(QPointer<COverlayBase> pOverlay);
  void RemoveOverlay(QPointer<COverlayBase> pOverlay);

private:
  std::vector<QPointer<COverlayBase>> m_vpOverlays;
};

#endif // OVERLAYMANAGER_H
