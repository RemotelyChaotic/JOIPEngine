#ifndef CMETRONOMEPAINTEDWIDGET_H
#define CMETRONOMEPAINTEDWIDGET_H

#include "Systems/DatabaseInterface/ProjectData.h"
#include "Systems/MetronomeManager.h"

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
  Q_PROPERTY(QStringList beatResources READ BeatResources WRITE SetBeatResources NOTIFY beatResourcesChanged)
  Q_PROPERTY(qint32  bpm READ Bpm WRITE SetBpm NOTIFY bpmChanged)
  Q_PROPERTY(qint32  metCmdMode READ MetCmdMode WRITE SetMetCmdMode NOTIFY metCmdModeChanged)
  Q_PROPERTY(bool    muted READ Muted WRITE SetMuted NOTIFY mutedChanged)
  Q_PROPERTY(QList<double> pattern READ Pattern WRITE SetPattern NOTIFY patternChanged)
  Q_PROPERTY(QColor  tickColor READ TickColor WRITE SetTickColor NOTIFY tickColorChanged)
  Q_PROPERTY(double  volume READ Volume WRITE SetVolume NOTIFY volumeChanged)

public:
  enum MetronomeStateChange
  {
    Started,
    Paused,
    Resumed,
    Stopped
  };
  Q_ENUM(MetronomeStateChange)

  CMetronomeCanvasQml(QQuickItem* pParent = nullptr);
  ~CMetronomeCanvasQml() override;

  void paint(QPainter* pPainter) override;

  QStringList BeatResources() const;
  void SetBeatResources(const QStringList& sResource);
  qint32 Bpm()const;
  void SetBpm(qint32 iValue);
  qint32 MetCmdMode() const;
  void SetMetCmdMode(qint32 iValue);
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
  void beatResourcesChanged();
  void bpmChanged();
  void metCmdModeChanged();
  void mutedChanged();
  void metronomeStateChanged(CMetronomeCanvasQml::MetronomeStateChange state);
  void patternChanged();
  void tickColorChanged();
  void tickReachedCenter();
  void volumeChanged();

private slots:
  void SlotTickReachedCenter(const CMetronomeManager::tId& id);
  void SlotUpdate(const CMetronomeManager::tId& id, const std::vector<double>& vdTicks);
  void SlotUpdateImpl(const std::vector<double>& vdTicks);

private:
  void UpdatePattern();

  std::shared_ptr<CMetronomeManager>           m_spMetronomeManager;
  std::shared_ptr<SMetronomeDataBlock>         m_spDataBlockThread;
  std::atomic<CMetronomeManager::tId>          m_id;
  QColor                                       m_tickColor;
  qint32                                       m_iBpm = 60;
  EToyMetronomeCommandModeFlags                m_metCmdMode = EToyMetronomeCommandModeFlag::eDefault;
  QList<double>                                m_vdPattern = { 1.0 };
  std::vector<double>                          m_vdTickmap;

  mutable QMutex                               m_localDataMutex;
  std::chrono::time_point<std::chrono::high_resolution_clock>
                                               m_lastUpdate;
};

#endif // CMETRONOMEPAINTEDWIDGET_H
