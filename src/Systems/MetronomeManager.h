#ifndef CMETRONOMEMANAGER_H
#define CMETRONOMEMANAGER_H

#include "ThreadedSystem.h"

#include <QFlags>
#include <QMutex>
#include <QTimer>
#include <QUuid>

#include <chrono>
#include <memory>

class CMultiEmitterSoundPlayer;
class CSettings;

//----------------------------------------------------------------------------------------
//
struct SMetronomeDataBlock
{
  SMetronomeDataBlock();
  SMetronomeDataBlock(const SMetronomeDataBlock& other);
  SMetronomeDataBlock& operator=(const SMetronomeDataBlock& other);

  mutable QMutex      m_mutex;

  QString             sUserName;

  QStringList         m_sBeatResources;
  bool                m_bMuted = false;
  double              m_dVolume = 1.0;

  std::vector<double> m_vdTickPattern;
};
struct SMetronomeDataBlockPrivate;

//----------------------------------------------------------------------------------------
//
enum ETickType
{
  // auditory ticks
  eSingle       = 0x01,
  ePattern      = 0x02,

  // toy ticks
  eVibrateTick  = 0x04,
  eLinearTick   = 0x08,
  eRotateTick   = 0x10
};
Q_DECLARE_FLAGS(ETickTypeFlags, ETickType)
Q_DECLARE_OPERATORS_FOR_FLAGS(ETickTypeFlags)
Q_DECLARE_METATYPE(ETickTypeFlags)

//----------------------------------------------------------------------------------------
//
class CMetronomeManager : public CSystemBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CMetronomeManager)

public:
  CMetronomeManager();
  ~CMetronomeManager() override;

  void DeregisterUi(const QUuid& sName);
  std::shared_ptr<SMetronomeDataBlock> RegisterUi(const QUuid& sName);
  QUuid IdForName(const QString& sName);
  void SpawnSingleBeat(const QString& sName);
  void SpawnSingleBeat(const QUuid& uid);

public slots:
  void Initialize() override;
  void Deinitialize() override;

signals:
  // incomming
  void SignalStart(const QUuid& id, ETickTypeFlags ticksToPlay);
  void SignalPause(const QUuid& id);
  void SignalResume(const QUuid& id);
  void SignalStop(const QUuid& id);
  void SignalBlockChanged(const QUuid& sName);
  // outgoing
  void SignalStarted(const QUuid& id);
  void SignalPaused(const QUuid& id);
  void SignalResumed(const QUuid& id);
  void SignalStopped(const QUuid& id);
  void SignalPatternChanged(const QUuid& id, const std::vector<double>& vdTicks);
  void SignalTickReachedCenter(const QUuid& id);

private slots:
  QUuid SlotIdForNameImpl(const QString& sName);
  void SlotSpawnSingleBeatImpl(const QString& sName);
  void SlotSpawnSingleBeatImpl(const QUuid& uid);
  void SlotStart(const QUuid& id, ETickTypeFlags ticksToPlay);
  void SlotPause(const QUuid& id);
  void SlotResume(const QUuid& id);
  void SlotStop(const QUuid& id);
  void SlotBlockChanged(const QUuid& sName);
  void SlotDeregisterUiImpl(const QUuid& sName);
  std::shared_ptr<SMetronomeDataBlock> SlotRegisterUiImpl(const QUuid& sName);
  void SlotTimeout();
  void SlotMutedChanged();
  void SlotVolumeChanged();

private:
  std::map<QUuid, std::shared_ptr<SMetronomeDataBlockPrivate>>   m_metronomeBlocks;
  std::shared_ptr<CSettings>                                     m_spSettings;
  QPointer<QTimer>                                               m_pTimer;
  std::chrono::time_point<std::chrono::high_resolution_clock>    m_lastUpdate;
  qint64                                                         m_iCumulativeTime = 0.0;
  double                                                         m_dVolume = 1.0;
  bool                                                           m_bMuted = false;
};

#endif // CMETRONOMEMANAGER_H
