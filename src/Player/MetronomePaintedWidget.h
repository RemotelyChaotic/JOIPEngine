#ifndef CMETRONOMEPAINTEDWIDGET_H
#define CMETRONOMEPAINTEDWIDGET_H

#include <QQuickPaintedItem>
#include <QMutex>
#include <QUuid>

#include <chrono>
#include <memory>
#include <vector>

class CMetronomeManager;
class CMultiEmitterSoundPlayer;
class CSettings;
struct SMetronomeDataBlock;
struct SResource;
typedef std::shared_ptr<SResource>      tspResource;

class CMetronomeCanvasQml : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QString beatResource READ BeatResource WRITE SetBeatResource NOTIFY beatResourceChanged)
  Q_PROPERTY(qint32  bpm READ Bpm WRITE SetBpm NOTIFY bpmChanged)
  Q_PROPERTY(bool    muted READ Muted WRITE SetMuted NOTIFY mutedChanged)
  Q_PROPERTY(QList<double> pattern READ Pattern WRITE SetPattern NOTIFY patternChanged)
  Q_PROPERTY(QColor  tickColor READ TickColor WRITE SetTickColor NOTIFY tickColorChanged)
  Q_PROPERTY(double  volume READ Volume WRITE SetVolume NOTIFY volumeChanged)

public:
  CMetronomeCanvasQml(QQuickItem* pParent = nullptr);
  ~CMetronomeCanvasQml() override;

  void paint(QPainter* pPainter) override;

  QString BeatResource() const;
  void SetBeatResource(const QString& sResource);
  qint32 Bpm()const;
  void SetBpm(qint32 iValue);
  bool Muted() const;
  void SetMuted(bool bMuted);
  QList<double> Pattern() const;
  void SetPattern(const QList<double>& vdVals);
  const QColor& TickColor() const;
  void SetTickColor(const QColor& color);
  double Volume() const;
  void SetVolume(double dValue);

public slots:
  void pause();
  void resume();
  void start();
  void stop();
  void registerUi(const QString& sUserName);

signals:
  void beatResourceChanged();
  void bpmChanged();
  void mutedChanged();
  void patternChanged();
  void tickColorChanged();
  void tickReachedCenter();
  void volumeChanged();

private slots:
  void SlotTickReachedCenter(const QUuid& id);
  void SlotUpdate(const QUuid& id, const std::vector<double>& vdTicks);
  void SlotUpdateImpl(const std::vector<double>& vdTicks);

private:
  void UpdatePattern();

  std::shared_ptr<CMetronomeManager>           m_spMetronomeManager;
  std::shared_ptr<SMetronomeDataBlock>         m_spDataBlockThread;
  QUuid                                        m_id;
  QColor                                       m_tickColor;
  qint32                                       m_iBpm = 60;
  QList<double>                                m_vdPattern = { 1.0 };
  std::vector<double>                          m_vdTickmap;

  mutable QMutex                               m_localDataMutex;
  std::chrono::time_point<std::chrono::high_resolution_clock>
                                               m_lastUpdate;
};

#endif // CMETRONOMEPAINTEDWIDGET_H
