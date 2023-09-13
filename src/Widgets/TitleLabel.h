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
  Q_PROPERTY(qint32 fontSize READ FontSize WRITE SetFontSize)

public:
  explicit CTitleLabel(QWidget* pParent = nullptr);
  explicit CTitleLabel(QString sText = "", QWidget* pParent = nullptr);

  void Initialize();
  void Invalidate();

  void SetOutlineColor(const QColor& color);
  const QColor& OutlineColor() const;
  void SetFontSize(qint32 iFontSize) { m_iFontSize = iFontSize; }
  qint32 FontSize() const { return m_iFontSize; }

  QSize SuggestedSize() const { return m_suggestedSize; }

protected slots:
  void SlotStyleLoaded();
  void SlotUpdate();

signals:
  void SignalAnimationFinished();

protected:
  void resizeEvent(QResizeEvent* pEvt) override;

private:
  void AddEffects();

  QPointer<CTitleProxyStyle> m_pStyle;
  QTimer                     m_updateTimer;
  QTimer                     m_settledTimer;
  QSize                      m_lastSize;
  QSize                      m_suggestedSize;
  qint32                     m_iFontSize;
  double                     m_dRatio;
  bool                       m_bUpdateGuard;
};

#endif // TITLELABEL_H
