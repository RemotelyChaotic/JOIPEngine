#ifndef CPOSITIONALMENU_H
#define CPOSITIONALMENU_H

#include <QFlag>
#include <QMenu>
#include <QShowEvent>

enum EMenuPopupPosition
{
  eTop = 0x1,
  eBottom = 0x2,
  eLeft = 0x4,
  eRight = 0x8
};

Q_DECLARE_FLAGS(EMenuPopupPositions, EMenuPopupPosition)
Q_DECLARE_OPERATORS_FOR_FLAGS(EMenuPopupPositions)

class CPositionalMenu : public QMenu
{
  Q_OBJECT
public:
  explicit CPositionalMenu(EMenuPopupPositions pos, QWidget* pParent = nullptr);
  explicit CPositionalMenu(const QString& sTitle, EMenuPopupPositions pos, QWidget* pParent = nullptr);
  ~CPositionalMenu();

protected:
  void showEvent(QShowEvent* event) override;

private:
  EMenuPopupPositions m_pos;
};

#endif // CPOSITIONALMENU_H
