#ifndef CMETRONOMEMANAGER_H
#define CMETRONOMEMANAGER_H

#include "ThreadedSystem.h"

#include <QFlags>
#include <QMutex>
#include <QTimer>

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
  using tId = quintptr;

  CMetronomeManager();
  ~CMetronomeManager() override;

  static tId GetNewId(void const* const ptr);

  void DeregisterUi(const tId& id);
  std::shared_ptr<SMetronomeDataBlock> RegisterUi(const tId& id);
  tId IdForName(const QString& sName);
  void SpawnSingleBeat(const QString& sName);
  void SpawnSingleBeat(const tId& uid);

public slots:
  void Initialize() override;
  void Deinitialize() override;

signals:
  // incomming
  void SignalStart(const CMetronomeManager::tId& id, ETickTypeFlags ticksToPlay);
  void SignalPause(const CMetronomeManager::tId& id);
  void SignalResume(const CMetronomeManager::tId& id);
  void SignalStop(const CMetronomeManager::tId& id);
  void SignalBlockChanged(const CMetronomeManager::tId& sName);
  // outgoing
  void SignalStarted(const CMetronomeManager::tId& id);
  void SignalPaused(const CMetronomeManager::tId& id);
  void SignalResumed(const CMetronomeManager::tId& id);
  void SignalStopped(const CMetronomeManager::tId& id);
  void SignalPatternChanged(const CMetronomeManager::tId& id, const std::vector<double>& vdTicks);
  void SignalTickReachedCenter(const CMetronomeManager::tId& id);

private slots:
  CMetronomeManager::tId SlotIdForNameImpl(const QString& sName);
  void SlotSpawnSingleBeatImpl(const QString& sName);
  void SlotSpawnSingleBeatImpl(const CMetronomeManager::tId& uid);
  void SlotStart(const CMetronomeManager::tId& id, ETickTypeFlags ticksToPlay);
  void SlotPause(const CMetronomeManager::tId& id);
  void SlotResume(const CMetronomeManager::tId& id);
  void SlotStop(const CMetronomeManager::tId& id);
  void SlotBlockChanged(const CMetronomeManager::tId& id);
  void SlotDeregisterUiImpl(const CMetronomeManager::tId& id);
  std::shared_ptr<SMetronomeDataBlock> SlotRegisterUiImpl(const CMetronomeManager::tId& id);
  void SlotTimeout();
  void SlotMutedChanged();
  void SlotVolumeChanged();

private:
  std::map<tId, std::shared_ptr<SMetronomeDataBlockPrivate>>     m_metronomeBlocks;
  std::shared_ptr<CSettings>                                     m_spSettings;
  QPointer<QTimer>                                               m_pTimer;
  std::chrono::time_point<std::chrono::high_resolution_clock>    m_lastUpdate;
  qint64                                                         m_iCumulativeTime = 0.0;
  double                                                         m_dVolume = 1.0;
  bool                                                           m_bMuted = false;
};

Q_DECLARE_METATYPE(CMetronomeManager::tId)

#endif // CMETRONOMEMANAGER_H
