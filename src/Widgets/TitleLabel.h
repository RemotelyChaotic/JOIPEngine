#ifndef TITLELABEL_H
#define TITLELABEL_H

#include <QLabel>
#include <QPointer>
#include <QTimer>

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
  void SlotUpdate();

protected:
  void resizeEvent(QResizeEvent* pEvt) override;

private:
  void AddEffects();

  QPointer<CTitleProxyStyle> m_pStyle;
  QTimer                     m_updateTimer;
};

#endif // TITLELABEL_H
