#ifndef CTIMERWIDGET_H
#define CTIMERWIDGET_H

#include <QWidget>

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
  friend class CTimerCanvas;

public:
  explicit CTimerWidget(QWidget* pParent = nullptr);
  ~CTimerWidget() override;

  void SetTimer(qint32 iTimeMs);
  void SetTimerVisible(bool bVisible);
  void SetUpdateInterval(qint32 iTimeMs);

public slots:
  void Update();

signals:
  void SignalTimerFinished();

protected:
  void resizeEvent(QResizeEvent* pEvent) override;

  QLabel*       m_pTimerBackGround;
  CTimerCanvas* m_pCanvas;
  QLabel*       m_pTimeLabel;
  QPixmap       m_bgImage;
  QPixmap       m_doubleBuffer;
  qint32        m_iTimeMsMax;
  qint32        m_iTimeMsCurrent;
  qint32        m_iUpdateInterval;
  qint32        m_iUpdateCounter;
  bool          m_bVisible;
};

#endif // CTIMERWIDGET_H
