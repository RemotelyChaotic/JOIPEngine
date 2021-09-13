#ifndef CMETRONOMEPAINTEDWIDGET_H
#define CMETRONOMEPAINTEDWIDGET_H

#include <QQuickPaintedItem>
#include <memory>
#include <vector>

class CMultiEmitterSoundPlayer;
class CSettings;
namespace QtAV { class AVPlayer; }
struct SResource;
typedef std::shared_ptr<SResource>      tspResource;

class CMetronomeCanvasQml : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QString beatResource READ BeatResource WRITE SetBeatResource NOTIFY beatResourceChanged)
  Q_PROPERTY(bool  muted READ Muted WRITE SetMuted NOTIFY mutedChanged)
  Q_PROPERTY(QColor  tickColor READ TickColor WRITE SetTickColor NOTIFY tickColorChanged)

public:
  CMetronomeCanvasQml(QQuickItem* pParent = nullptr);
  ~CMetronomeCanvasQml() override;

  void paint(QPainter* pPainter) override;

  const QString& BeatResource() const;
  void SetBeatResource(const QString& sResource);
  bool Muted() const;
  void SetMuted(bool bMuted);
  const QColor& TickColor() const;
  void SetTickColor(const QColor& color);

public slots:
  void clear();
  void spawnNewMetronomeTicks();
  void update(double dIntervalMs);

signals:
  void beatResourceChanged();
  void mutedChanged();
  void tickColorChanged();
  void tickReachedCenter();

private slots:
  void SlotMutedChanged();
  void SlotVolumeChanged();

private:
  std::unique_ptr<CMultiEmitterSoundPlayer>    m_spSoundEmitters;
  std::shared_ptr<CSettings>                   m_spSettings;
  QString                                      m_sBeatResource;
  QColor                                       m_tickColor;
  std::vector<double>                          m_vdTickmap;
  bool                                         m_bMuted;
};

#endif // CMETRONOMEPAINTEDWIDGET_H
