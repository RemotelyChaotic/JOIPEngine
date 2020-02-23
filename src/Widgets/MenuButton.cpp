#include "MenuButton.h"
#include "Application.h"

#include <QFont>

CMenuButton::CMenuButton(QWidget* pParent) :
  QPushButton(pParent)
{
  connect(CApplication::Instance(), &CApplication::StyleLoaded,
          this, &CMenuButton::SlotStyleLoaded, Qt::QueuedConnection);

  QFont thisFont = this->font();
  thisFont.setPixelSize(24);
  setFont(thisFont);
}

void CMenuButton::SlotStyleLoaded()
{
  QFont thisFont = this->font();
  thisFont.setPixelSize(24);
  thisFont.setFamily(CApplication::Instance()->Settings()->Font());
  setFont(thisFont);
}
