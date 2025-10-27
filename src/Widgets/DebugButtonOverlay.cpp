#include "DebugButtonOverlay.h"

CDebugButtonOverlay::CDebugButtonOverlay(QWidget* pParent) :
  COverlayButton(QStringLiteral("DebugLogButtonOverlay"),
                 QStringLiteral("DebugLogButton"),
                 QStringLiteral("Open debug log"),
                 QStringLiteral("Debug"), pParent)
{}
CDebugButtonOverlay::~CDebugButtonOverlay() = default;

//----------------------------------------------------------------------------------------
//
void CDebugButtonOverlay::Hide()
{
  COverlayButton::Hide();
}

//----------------------------------------------------------------------------------------
//
void CDebugButtonOverlay::Resize()
{
  QPoint pos{50, 110};
  QSize parentSize = m_pTargetWidget->size();
  move(parentSize.width() - pos.x() - sizeHint().width(), pos.y());
}

//----------------------------------------------------------------------------------------
//
void CDebugButtonOverlay::Show()
{
  COverlayButton::Show();
}
