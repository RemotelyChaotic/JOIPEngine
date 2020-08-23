#ifndef CMETRONOMEPAINTEDWIDGET_H
#define CMETRONOMEPAINTEDWIDGET_H

#include <QQuickPaintedItem>
#include <memory>
#include <vector>

namespace QtAV { class AVPlayer; }
struct SResource;
typedef std::shared_ptr<SResource>      tspResource;

class CMetronomeCanvasQml : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QString beatResource READ BeatResource WRITE SetBeatResource NOTIFY beatResourceChanged)
  Q_PROPERTY(QColor  tickColor READ TickColor WRITE SetTickColor NOTIFY tickColorChanged)

public:
  CMetronomeCanvasQml(QQuickItem* pParent = nullptr);
  ~CMetronomeCanvasQml() override;

  void paint(QPainter* pPainter) override;

  const QString& BeatResource() const;
  void SetBeatResource(const QString& sResource);
  const QColor& TickColor() const;
  void SetTickColor(const QColor& color);

public slots:
  void spawnNewMetronomeTicks();
  void update(double dIntervalMs);

signals:
  void beatResourceChanged();
  void tickColorChanged();
  void tickReachedCenter();

private:
  std::unique_ptr<QtAV::AVPlayer> m_player;
  QString                         m_sBeatResource;
  QColor                          m_tickColor;
  std::vector<double>             m_vdTickmap;
};

#endif // CMETRONOMEPAINTEDWIDGET_H
