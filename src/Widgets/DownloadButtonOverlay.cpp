#include "DownloadButtonOverlay.h"

CDownloadButtonOverlay::CDownloadButtonOverlay(QWidget* pParent) :
  COverlayButton(QStringLiteral("DownloadButtonOverlay"),
                 QStringLiteral("DownloadButton"),
                 QStringLiteral("Open download page"),
                 QStringLiteral("Download"), pParent)
{

}

CDownloadButtonOverlay::~CDownloadButtonOverlay()
{

}

//----------------------------------------------------------------------------------------
//
void CDownloadButtonOverlay::Resize()
{
  move(50, 50);
}
