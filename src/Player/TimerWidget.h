#ifndef CTIMERWIDGET_H
#define CTIMERWIDGET_H

#include <QWidget>
#include <chrono>
#include <memory>

class CSettings;
class CTimerWidget;
class QLabel;
class QSvgRenderer;

class CTimerCanvas : public QWidget
{
  Q_OBJECT

public:
  explicit CTimerCanvas(CTimerWidget* pParent = nullptr);
  ~CTimerCanvas() override;

protected:
  void paintEvent(QPaintEvent* pEvent) override;

private:
  CTimerWidget*         m_pParent;
};


class CTimerWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QColor primaryColor READ PrimaryColor WRITE SetPrimaryColor)
  Q_PROPERTY(QColor secondaryColor READ SecondaryColor WRITE SetSecondaryColor)
  Q_PROPERTY(QColor tertiaryColor READ TertiaryColor WRITE SetTertiaryColor)
  friend class CTimerCanvas;

public:
  explicit CTimerWidget(QWidget* pParent = nullptr);
  ~CTimerWidget() override;

  void SetTimer(qint32 iTimeMs);
  void SetTimerVisible(bool bVisible);
  void SetUpdateInterval(qint32 iTimeMs);

  void SetPrimaryColor(const QColor& color);
  const QColor& PrimaryColor();
  void SetSecondaryColor(const QColor& color);
  const QColor& SecondaryColor();
  void SetTertiaryColor(const QColor& color);
  const QColor& TertiaryColor();

public slots:
  void Update();

signals:
  void SignalTimerFinished();

protected:
  void resizeEvent(QResizeEvent* pEvent) override;

protected slots:
  void SlotFontChanged();

protected:
  using Timestamp = std::chrono::steady_clock::time_point;

  std::shared_ptr<CSettings>m_spSettings;
  QLabel*                   m_pTimerBackGround;
  CTimerCanvas*             m_pCanvas;
  QLabel*                   m_pTimeLabel;
  QPixmap                   m_bgImage;
  QPixmap                   m_doubleBuffer;
  QColor                    m_primaryColor;
  QColor                    m_secondaryColor;
  QColor                    m_tertiaryColor;
  Timestamp                 m_lastUpdateTime;
  qint32                    m_iTimeMsMax;
  qint32                    m_iTimeMsCurrent;
  qint32                    m_iUpdateInterval;
  qint32                    m_iUpdateCounter;
  bool                      m_bVisible;
};

#endif // CTIMERWIDGET_H
