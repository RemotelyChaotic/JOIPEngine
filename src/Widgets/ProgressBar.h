#ifndef CPROGRESSBAR_H
#define CPROGRESSBAR_H

#include <QPointer>
#include <QProgressBar>

class QPropertyAnimation;

void PaintProgress(QPainter* pPainter, QColor primaryColor, QColor secondaryColor, QColor tertiaryColor,
                   qint32 iBorderWidth, qint32 iGroveWidth,
                   qint32 iWidth, qint32 iHeight, QRect contentsRect,
                   qint32 iTimeMsMax, qint32 iTimeMsCurrent,
                   qint32 iUpdateCounter, bool bVisibleCounter,
                   bool bDrawDecoration = true, bool bFromQml = false);

class CProgressBar : public QProgressBar
{
  Q_OBJECT
  Q_PROPERTY(QColor primaryColor READ PrimaryColor WRITE SetPrimaryColor NOTIFY primaryColorChanged)
  Q_PROPERTY(QColor secondaryColor READ SecondaryColor WRITE SetSecondaryColor NOTIFY secondaryColorChanged)
  Q_PROPERTY(QColor tertiaryColor READ TertiaryColor WRITE SetTertiaryColor NOTIFY tertiaryColorColorChanged)
  Q_PROPERTY(bool   drawDecoration MEMBER m_bDrawDecoration)
  Q_PROPERTY(qint32 iUpdateCounter MEMBER m_iUpdateCounter)

public:
  static qint32 c_iBorderWidth;
  static qint32 c_iGroveWidth;

  CProgressBar(QWidget* pParent = nullptr);
  ~CProgressBar() override;

  void SetPrimaryColor(const QColor& color);
  const QColor& PrimaryColor();
  void SetSecondaryColor(const QColor& color);
  const QColor& SecondaryColor();
  void SetTertiaryColor(const QColor& color);
  const QColor& TertiaryColor();

  void SetRange(qint32 iMinimum, qint32 iMaximum);

signals:
  void primaryColorChanged();
  void secondaryColorChanged();
  void tertiaryColorColorChanged();

protected slots:
  void SlotAnimationUpdate();

protected:
  void paintEvent(QPaintEvent* pPainter) override;

private:
  QPointer<QPropertyAnimation> m_pIdleAnim;
  QColor                       m_primaryColor;
  QColor                       m_secondaryColor;
  QColor                       m_tertiaryColor;
  qint32                       m_iUpdateCounter;
  bool                         m_bAnimationRunning;
  bool                         m_bDrawDecoration;
};

#endif // CPROGRESSBAR_H
