#ifndef CMINIMIZINGSCROLLAREA_H
#define CMINIMIZINGSCROLLAREA_H

#include <QScrollArea>

class CMinimizingScrollArea : public QScrollArea
{
  Q_OBJECT
public:
  explicit CMinimizingScrollArea(QWidget* pParent = nullptr);

protected:
  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;
};

#endif // CMINIMIZINGSCROLLAREA_H
