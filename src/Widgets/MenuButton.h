#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include <QPushButton>

class CMenuButton : public QPushButton
{
  Q_OBJECT
public:
  explicit CMenuButton(QWidget* pParent = nullptr);
};

#endif // MENUBUTTON_H
