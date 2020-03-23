#include "OverlayManager.h"
#include "Widgets/OverlayBase.h"
#include <map>

COverlayManager::COverlayManager() :
  CSystemBase()
{

}
COverlayManager::~COverlayManager()
{

}

//----------------------------------------------------------------------------------------
//
void COverlayManager::Initialize()
{
  m_vpOverlays.clear();
  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void COverlayManager::Deinitialize()
{
  m_vpOverlays.clear();
  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
void COverlayManager::RebuildOverlayOrder()
{
  if (!IsInitialized()) { return; }

  std::map<QWidget*, std::vector<QPointer<COverlayBase>>> builtParentMap;

  // build map ordered by parent of overlay
  for (auto pOverlay : m_vpOverlays)
  {
    auto& vpOverlays = builtParentMap[qobject_cast<QWidget*>(pOverlay->parent())];
    vpOverlays.push_back(pOverlay);
  }

  // order vectors, ordering by ZOrder
  for (auto it = builtParentMap.begin(); builtParentMap.end() != it; ++it)
  {
    if (it->second.size() < 2) continue;

    std::sort(it->second.begin(), it->second.end(), [](QPointer<COverlayBase> a,
                                                       QPointer<COverlayBase> b) {
      return !a.isNull() && !b.isNull() && (a->ZOrder() < b->ZOrder());
    });
    // raise in order
    for (QPointer<COverlayBase> pOverlay : it->second)
    {
      pOverlay->raise();
    }
  }
}

//----------------------------------------------------------------------------------------
//
void COverlayManager::RegisterOverlay(QPointer<COverlayBase> pOverlay)
{
  if (!IsInitialized()) { return; }

  auto it = std::find(m_vpOverlays.begin(), m_vpOverlays.end(), pOverlay);
  if (m_vpOverlays.end() == it)
  {
    m_vpOverlays.push_back(pOverlay);
  }
}

//----------------------------------------------------------------------------------------
//
void COverlayManager::RemoveOverlay(QPointer<COverlayBase> pOverlay)
{
  if (!IsInitialized()) { return; }

  auto it = std::find(m_vpOverlays.begin(), m_vpOverlays.end(), pOverlay);
  if (m_vpOverlays.end() != it)
  {
    m_vpOverlays.erase(it);
  }
}
