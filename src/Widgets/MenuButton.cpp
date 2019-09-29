#include "MenuButton.h"

#include <QFont>

CMenuButton::CMenuButton(QWidget* pParent) :
  QPushButton(pParent)
{
  QFont thisFont = this->font();
  thisFont.setPixelSize(24);
  setFont(thisFont);
}
