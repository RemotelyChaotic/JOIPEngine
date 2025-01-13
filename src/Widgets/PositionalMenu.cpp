#include "PositionalMenu.h"

CPositionalMenu::CPositionalMenu(EMenuPopupPositions pos, QWidget *parent) :
  QMenu{parent},
  m_pos(pos)
{
}
CPositionalMenu::CPositionalMenu(const QString& sTitle, EMenuPopupPositions pos, QWidget* pParent):
  QMenu{sTitle, pParent},
  m_pos(pos)
{
}
CPositionalMenu::~CPositionalMenu() = default;

//----------------------------------------------------------------------------------------
//
void CPositionalMenu::showEvent(QShowEvent* event)
{
  Q_UNUSED(event)

  QPoint p = this->pos();
  if (m_pos.testFlag(EMenuPopupPosition::eTop))
  {
    p.setY(p.y()- height());
  }
  if (m_pos.testFlag(EMenuPopupPosition::eLeft))
  {
    p.setX(p.x()- width());
  }

  move(p.x(), p.y());
  resize(sizeHint());
}
