#ifndef TITLELABEL_H
#define TITLELABEL_H

#include <QLabel>
#include <QPointer>

class CTitleProxyStyle;

class CTitleLabel : public QLabel
{
  Q_OBJECT
  Q_PROPERTY(QColor outlineColor READ OutlineColor WRITE SetOutlineColor)

public:
  explicit CTitleLabel(QWidget* pParent = nullptr);
  explicit CTitleLabel(QString sText = "", QWidget* pParent = nullptr);

  void SetOutlineColor(const QColor& color);
  const QColor& OutlineColor();

protected slots:
  void SlotStyleLoaded();

private:
  void AddEffects();

  QPointer<CTitleProxyStyle> m_pStyle;
};

#endif // TITLELABEL_H
