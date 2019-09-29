#ifndef TITLELABEL_H
#define TITLELABEL_H

#include <QLabel>

class CTitleLabel : public QLabel
{
  Q_OBJECT
public:
  explicit CTitleLabel(QWidget* pParent = nullptr);
  explicit CTitleLabel(QString sText = "", QWidget* pParent = nullptr);

private:
  void AddEffects();
};

#endif // TITLELABEL_H
