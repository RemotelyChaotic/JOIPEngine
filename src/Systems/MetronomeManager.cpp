#include "MetronomeManager.h"
#include "Application.h"
#include "Settings.h"

#include "Systems/DeviceManager.h"
#include "Systems/Devices/IDevice.h"
#include "Systems/Resource.h"

#include "Utils/MetronomeHelpers.h"
#include "Utils/MultiEmitterSoundPlayer.h"

#include <QTimer>

namespace
{
  // we want a high update resolution, but only want to update the UI
  // at 60-90 fps to reduce system load
  constexpr qint32 c_iTimerInterval = 8; // ~120fps
  constexpr qint32 c_iUpdateThresholdInterval = 12; // 1.5* c_iTimerInterval; ~60fps == 16ms
}

Q_DECLARE_METATYPE(std::shared_ptr<SMetronomeDataBlock>)

//----------------------------------------------------------------------------------------
//
SMetronomeDataBlock::SMetronomeDataBlock() :
  m_mutex(QMutex::Recursive)
{
}
SMetronomeDataBlock::SMetronomeDataBlock(const SMetronomeDataBlock& other)
{
  *this = other;
}
SMetronomeDataBlock& SMetronomeDataBlock::operator=(const SMetronomeDataBlock& other)
{
  sUserName = other.sUserName;
  m_sBeatResource = other.m_sBeatResource;
  m_bMuted = other.m_bMuted;
  m_dVolume = other.m_dVolume;
  m_vdTickPattern = other.m_vdTickPattern;
  return *this;
}

struct SMetronomeTick
{
  ETickType type;
  double dTimePos;
  qint32 iData = -1;
};

struct SMetronomeDataBlockImpl : public SMetronomeDataBlock
{
  SMetronomeDataBlockImpl() :
      SMetronomeDataBlock()
  {}
  SMetronomeDataBlockImpl& operator=(const SMetronomeDataBlock& other)
  {
    SMetronomeDataBlock::operator=(other);
    return *this;
  }

  std::unique_ptr<CMultiEmitterSoundPlayer> m_spSoundEmitters;
  std::shared_ptr<IDevice>    m_spCurrentDevice;
  std::optional<SMetronomeTick> m_lastDeviceTick;
  std::vector<SMetronomeTick> m_vdSpawnedTicks;
  ETickTypeFlags m_playFlags;
  bool m_lastTickHigh = false;
  bool m_bStarted = false;
  bool m_bPaused = false;
};
struct SMetronomeDataBlockPrivate
{
  SMetronomeDataBlockPrivate() :
      m_spPublicBlock(std::make_shared<SMetronomeDataBlock>())
  { }

  std::shared_ptr<SMetronomeDataBlock> m_spPublicBlock;
  SMetronomeDataBlockImpl m_privateBlock; // mutex is not used here
};

namespace
{
  void RunDeviceTick(const SMetronomeTick& tick,
                     std::shared_ptr<IDevice> spDevice)
  {
    if (nullptr != spDevice)
    {
      if (ETickType::eVibrateTick == tick.type)
      {
        spDevice->SendVibrateCmd(
            double(tick.iData) / 100.0);
      }
      else if (ETickType::eLinearTick == tick.type)
      {
        const quint32 iDuration = tick.iData/1000;
        const double dPosition = double(tick.iData%1000) / 100.0;
        spDevice->SendLinearCmd(iDuration, dPosition);
      }
      else if (ETickType::eRotateTick == tick.type)
      {
        const bool bClockWise = tick.iData > 0;
        const double dSpeed = double(std::abs(tick.iData)) / 100.0;
        spDevice->SendRotateCmd(bClockWise, dSpeed);
      }
    }
  }
}


//----------------------------------------------------------------------------------------
//
CMetronomeManager::CMetronomeManager() :
  CSystemBase(QThread::Priority::HighPriority), // the high thread priority is important otherwise
                                                // the update might starve for too long and we
                                                // get a choppy metronome
  m_spSettings(CApplication::Instance()->Settings())
{
  qRegisterMetaType<std::shared_ptr<SMetronomeDataBlock>>();
  qRegisterMetaType<std::vector<double>>();
  qRegisterMetaType<ETickTypeFlags>();

  connect(this, &CMetronomeManager::SignalStart,
          this, &CMetronomeManager::SlotStart, Qt::QueuedConnection);
  connect(this, &CMetronomeManager::SignalPause,
          this, &CMetronomeManager::SlotPause, Qt::QueuedConnection);
  connect(this, &CMetronomeManager::SignalResume,
          this, &CMetronomeManager::SlotResume, Qt::QueuedConnection);
  connect(this, &CMetronomeManager::SignalStop,
          this, &CMetronomeManager::SlotStop, Qt::QueuedConnection);
  connect(this, &CMetronomeManager::SignalBlockChanged,
          this, &CMetronomeManager::SlotBlockChanged, Qt::QueuedConnection);
}
CMetronomeManager::~CMetronomeManager()
{
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::DeregisterUi(const QUuid& sName)
{
  bool bOk = QMetaObject::invokeMethod(this, "SlotDeregisterUiImpl",
                                       Qt::QueuedConnection,
                                       Q_ARG(QUuid, sName));
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SMetronomeDataBlock> CMetronomeManager::RegisterUi(const QUuid& sName)
{
  std::shared_ptr<SMetronomeDataBlock> spRet;
  bool bOk = QMetaObject::invokeMethod(this, "SlotRegisterUiImpl",
                                       Qt::BlockingQueuedConnection,
                                       Q_RETURN_ARG(std::shared_ptr<SMetronomeDataBlock>, spRet),
                                       Q_ARG(QUuid, sName));
  assert(bOk); Q_UNUSED(bOk)
  return spRet;
}

//----------------------------------------------------------------------------------------
//
QUuid CMetronomeManager::IdForName(const QString& sName)
{
  QUuid retId;
  bool bOk = QMetaObject::invokeMethod(this, "SlotIdForNameImpl",
                                       Qt::BlockingQueuedConnection,
                                       Q_RETURN_ARG(QUuid, retId),
                                       Q_ARG(QString, sName));
  assert(bOk); Q_UNUSED(bOk)
  return retId;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SpawnSingleBeat(const QString& sName)
{
  bool bOk = QMetaObject::invokeMethod(this, "SlotSpawnSingleBeatImpl",
                                       Qt::QueuedConnection,
                                       Q_ARG(QString, sName));
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SpawnSingleBeat(const QUuid& uid)
{
  bool bOk = QMetaObject::invokeMethod(this, "SlotSpawnSingleBeatImpl",
                                       Qt::QueuedConnection,
                                       Q_ARG(QUuid, uid));
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::Initialize()
{
  if (nullptr != m_spSettings)
  {
    connect(m_spSettings.get(), &CSettings::mutedChanged,
            this, &CMetronomeManager::SlotMutedChanged, Qt::QueuedConnection);
    connect(m_spSettings.get(), &CSettings::volumeChanged,
            this, &CMetronomeManager::SlotVolumeChanged, Qt::QueuedConnection);
  }

  SlotMutedChanged();
  SlotVolumeChanged();

  m_pTimer = new QTimer(this);
  // the timer has to be precise, otherwise we get jumps in the metronome
  m_pTimer->setTimerType(Qt::TimerType::PreciseTimer);
  m_pTimer->setInterval(std::chrono::milliseconds(c_iTimerInterval));
  m_pTimer->setSingleShot(false);
  connect(m_pTimer, &QTimer::timeout, this, &CMetronomeManager::SlotTimeout);

  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::Deinitialize()
{
  m_pTimer->stop();
  delete m_pTimer;

  SetInitialized(false);
}

//----------------------------------------------------------------------------------------
//
QUuid CMetronomeManager::SlotIdForNameImpl(const QString& sName)
{
  auto it = std::find_if(m_metronomeBlocks.begin(), m_metronomeBlocks.end(),
                         [&sName](const std::pair<QUuid, std::shared_ptr<SMetronomeDataBlockPrivate>>& pair) {
    return pair.second->m_privateBlock.sUserName == sName;
  });
  if (m_metronomeBlocks.end() != it)
  {
    return it->first;
  }
  return QUuid();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotSpawnSingleBeatImpl(const QString& sName)
{
  auto it = std::find_if(m_metronomeBlocks.begin(), m_metronomeBlocks.end(),
                         [&sName](const std::pair<QUuid, std::shared_ptr<SMetronomeDataBlockPrivate>>& pair) {
    return pair.second->m_privateBlock.sUserName == sName;
  });
  if (m_metronomeBlocks.end() != it)
  {
    it->second->m_privateBlock.m_vdSpawnedTicks.push_back({ETickType::eSingle, 0.0, -1});
    std::sort(it->second->m_privateBlock.m_vdSpawnedTicks.begin(),
              it->second->m_privateBlock.m_vdSpawnedTicks.end(),
              [](const SMetronomeTick& pairA,
                 const SMetronomeTick& pairB) {
      return pairA.dTimePos < pairB.dTimePos;
    });
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotSpawnSingleBeatImpl(const QUuid& uid)
{
  auto it = m_metronomeBlocks.find(uid);
  if (m_metronomeBlocks.end() != it)
  {
    it->second->m_privateBlock.m_vdSpawnedTicks.push_back({ETickType::eSingle, 0.0, -1});
    std::sort(it->second->m_privateBlock.m_vdSpawnedTicks.begin(),
              it->second->m_privateBlock.m_vdSpawnedTicks.end(),
              [](const SMetronomeTick& pairA,
                 const SMetronomeTick& pairB) {
      return pairA.dTimePos < pairB.dTimePos;
    });
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotStart(const QUuid& id, ETickTypeFlags ticksToPlay)
{
  auto it = m_metronomeBlocks.find(id);
  if (m_metronomeBlocks.end() != it)
  {
    it->second->m_privateBlock.m_playFlags |= ticksToPlay;

    if (!it->second->m_privateBlock.m_bStarted)
    {
      it->second->m_privateBlock.m_bStarted = true;
      it->second->m_privateBlock.m_lastTickHigh = false;

      if (nullptr != m_pTimer && !m_pTimer->isActive())
      {
        // find active device
        if (auto spDeviceMan = CApplication::Instance()->System<CDeviceManager>().lock())
        {
          it->second->m_privateBlock.m_spCurrentDevice =
              spDeviceMan->Device(spDeviceMan->SelectedDevice());
        }
        else
        {
          it->second->m_privateBlock.m_spCurrentDevice = nullptr;
        }

        it->second->m_privateBlock.m_lastDeviceTick = std::nullopt;

        m_pTimer->start();
        m_lastUpdate = std::chrono::high_resolution_clock::now();
        m_iCumulativeTime = 0.0;
      }

      emit SignalStarted(id);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotPause(const QUuid& id)
{
  auto it = m_metronomeBlocks.find(id);
  if (m_metronomeBlocks.end() != it)
  {
    if (it->second->m_privateBlock.m_bStarted)
    {
      if (nullptr != it->second->m_privateBlock.m_spCurrentDevice)
      {
        it->second->m_privateBlock.m_spCurrentDevice->SendStopCmd();
      }

      it->second->m_privateBlock.m_bPaused = true;
      emit SignalPaused(id);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotResume(const QUuid& id)
{
  auto it = m_metronomeBlocks.find(id);
  if (m_metronomeBlocks.end() != it)
  {
    if (it->second->m_privateBlock.m_bStarted)
    {
      // refecth active device
      if (auto spDeviceMan = CApplication::Instance()->System<CDeviceManager>().lock())
      {
        it->second->m_privateBlock.m_spCurrentDevice =
            spDeviceMan->Device(spDeviceMan->SelectedDevice());

        if (it->second->m_privateBlock.m_lastDeviceTick.has_value())
        {
          RunDeviceTick(it->second->m_privateBlock.m_lastDeviceTick.value(),
                        it->second->m_privateBlock.m_spCurrentDevice);

          it->second->m_privateBlock.m_lastDeviceTick = std::nullopt;
        }
      }
      else
      {
        it->second->m_privateBlock.m_spCurrentDevice = nullptr;
      }
      it->second->m_privateBlock.m_bPaused = false;
      emit SignalResumed(id);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotStop(const QUuid& id)
{
  auto it = m_metronomeBlocks.find(id);
  if (m_metronomeBlocks.end() != it)
  {
    if (it->second->m_privateBlock.m_bStarted)
    {
      if (nullptr != it->second->m_privateBlock.m_spCurrentDevice)
      {
        it->second->m_privateBlock.m_spCurrentDevice->SendStopCmd();
      }

      it->second->m_privateBlock.m_lastTickHigh = false;
      it->second->m_privateBlock.m_bStarted = false;
      it->second->m_privateBlock.m_vdSpawnedTicks.clear();
      it->second->m_privateBlock.m_spCurrentDevice = nullptr;
      it->second->m_privateBlock.m_lastDeviceTick = std::nullopt;
      it->second->m_privateBlock.m_playFlags = 0;

      emit SignalStopped(id);

      bool bAnyRunning = false;
      for (const auto& [_, block] : m_metronomeBlocks)
      {
        bAnyRunning |= block->m_privateBlock.m_bStarted;
      }

      if (!bAnyRunning && nullptr != m_pTimer && m_pTimer->isActive())
      {
        m_pTimer->stop();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotBlockChanged(const QUuid& sName)
{
  auto it = m_metronomeBlocks.find(sName);
  if (m_metronomeBlocks.end() != it)
  {
    {
      QMutexLocker locker(&it->second->m_spPublicBlock->m_mutex);
      it->second->m_privateBlock = *it->second->m_spPublicBlock;
    }

    if (it->second->m_privateBlock.m_sBeatResource.isEmpty())
    {
      if (nullptr != m_spSettings)
      {
        it->second->m_privateBlock.m_spSoundEmitters->SetSoundEffect(
            metronome::MetronomeSfxFromKey(m_spSettings->MetronomeSfx()));
      }
    }
    else
    {
      it->second->m_privateBlock.m_spSoundEmitters->SetSoundEffect(
          it->second->m_privateBlock.m_sBeatResource);
    }

    it->second->m_privateBlock.m_spSoundEmitters->SetMuted(
        it->second->m_privateBlock.m_bMuted || m_bMuted);
    it->second->m_privateBlock.m_spSoundEmitters->SetVolume(
        it->second->m_privateBlock.m_dVolume * m_dVolume);
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotDeregisterUiImpl(const QUuid& sName)
{
  auto it = m_metronomeBlocks.find(sName);
  if (m_metronomeBlocks.end() != it)
  {
    SlotStop(sName);
    m_metronomeBlocks.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SMetronomeDataBlock> CMetronomeManager::SlotRegisterUiImpl(const QUuid& sName)
{
  m_metronomeBlocks[sName] = std::make_shared<SMetronomeDataBlockPrivate>();
  auto block = m_metronomeBlocks[sName];

  if (nullptr != m_spSettings)
  {
    block->m_spPublicBlock->m_sBeatResource =
        metronome::MetronomeSfxFromKey(m_spSettings->MetronomeSfx());
    block->m_spPublicBlock->m_bMuted = m_bMuted;
    block->m_spPublicBlock->m_dVolume = m_dVolume;
  }

  block->m_privateBlock = *block->m_spPublicBlock;
  block->m_privateBlock.m_spSoundEmitters =
      std::make_unique<CMultiEmitterSoundPlayer>(CMultiEmitterSoundPlayer::c_iDefaultNumAutioEmitters,
                                                 block->m_spPublicBlock->m_sBeatResource);
  return block->m_spPublicBlock;
}

//----------------------------------------------------------------------------------------
//
namespace
{
  bool NeedsToSpawnTicks(const std::vector<SMetronomeTick>& vdSpawnedTicks,
                         const ETickTypeFlags& ticksToPlay,
                         double& dLastPatternTick,
                         bool& bFoundLastPatternTick)
  {
    bFoundLastPatternTick = false;
    if (!ticksToPlay.testFlag(ETickType::ePattern)) { return false; }

    bool bRet = vdSpawnedTicks.empty();
    dLastPatternTick = 1.0;
    if (!bRet)
    {
      for (auto it = vdSpawnedTicks.rbegin(); vdSpawnedTicks.rend() != it; ++it)
      {
        if (ETickType::ePattern == it->type)
        {
          bFoundLastPatternTick = true;
          dLastPatternTick = it->dTimePos;
          return dLastPatternTick >= 0.0;
        }
      }
    }
    return bRet;
  }

  //--------------------------------------------------------------------------------------
  //
  std::vector<double> TransformTicksForUi(const std::vector<SMetronomeTick>& vdSpawnedTicks)
  {
    // only these two flags have a visual representation for now
    constexpr ETickTypeFlags c_mask = ETickType::eSingle | ETickType::ePattern;
    std::vector<double> vdRet;
    for (const auto& tick : vdSpawnedTicks)
    {
      if (c_mask.testFlag(tick.type))
      {
        vdRet.push_back(tick.dTimePos);
      }
    }
    return vdRet;
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotTimeout()
{
  constexpr ETickTypeFlags c_maskAudio = ETickType::eSingle | ETickType::ePattern;

  auto now = std::chrono::high_resolution_clock::now();
  qint64 iDiffMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdate).count();
  m_lastUpdate = now;
  m_iCumulativeTime += iDiffMs;

  for (const auto& [id, spBlock] : m_metronomeBlocks)
  {
    if (spBlock->m_privateBlock.m_bStarted && !spBlock->m_privateBlock.m_bPaused)
    {
      for (qint32 i = 0; spBlock->m_privateBlock.m_vdSpawnedTicks.size() > static_cast<size_t>(i); ++i)
      {
        SMetronomeTick& tick =
            spBlock->m_privateBlock.m_vdSpawnedTicks[static_cast<size_t>(i)];
        // 1s to reach end
        tick.dTimePos += static_cast<double>(iDiffMs / 1'000.0);
        if (tick.dTimePos >= 1.0)
        {
          // play tick audio if type is auditory
          if (c_maskAudio.testFlag(tick.type))
          {
            spBlock->m_privateBlock.m_spSoundEmitters->Play();
            emit SignalTickReachedCenter(id);
          }

          // run toy commands
          RunDeviceTick(tick, spBlock->m_privateBlock.m_spCurrentDevice);
          spBlock->m_privateBlock.m_lastDeviceTick = tick;

          spBlock->m_privateBlock.m_vdSpawnedTicks.erase(
              spBlock->m_privateBlock.m_vdSpawnedTicks.begin()+static_cast<size_t>(i));
          --i;
        }
      }

      // spawn repeating ticks
      bool bFoundLastPatternTick = false;
      double dLastPatternTick = 0.0;
      if (NeedsToSpawnTicks(spBlock->m_privateBlock.m_vdSpawnedTicks,
                            spBlock->m_privateBlock.m_playFlags,
                            dLastPatternTick, bFoundLastPatternTick))
      {
        for (size_t i = 0; spBlock->m_privateBlock.m_vdTickPattern.size() > i; ++i)
        {
          auto dDistMsTick = spBlock->m_privateBlock.m_vdTickPattern[i];
          double dLastTickLoc = dLastPatternTick - static_cast<double>(dDistMsTick / 1'000.0);
          if (bFoundLastPatternTick)
          {
            qint32 iDistEncoded = 100+static_cast<qint32>(dDistMsTick/2*1000)/1000*1000;
            if (spBlock->m_privateBlock.m_playFlags.testFlag(ETickType::eVibrateTick))
              spBlock->m_privateBlock.m_vdSpawnedTicks.push_back(SMetronomeTick{
                ETickType::eVibrateTick, dLastPatternTick-dLastTickLoc/2, 25});
            if (spBlock->m_privateBlock.m_playFlags.testFlag(ETickType::eLinearTick))
              spBlock->m_privateBlock.m_vdSpawnedTicks.push_back(SMetronomeTick{
                ETickType::eLinearTick, dLastPatternTick-dLastTickLoc/2, iDistEncoded});
          }

          // + -> clockwise, - -> anti-clockwise, abs() -> speed
          qint32 iLastRotateData = spBlock->m_privateBlock.m_lastTickHigh ? 25 : -25;
          // iDistEncoded = e.g.: 100'000 -> (ms)(ms)(ms)'(pos)(pos)(pos)
          qint32 iDistEncoded = 0+static_cast<qint32>(dDistMsTick/2*1000)/1000*1000;

          spBlock->m_privateBlock.m_vdSpawnedTicks.push_back(SMetronomeTick{
              ETickType::ePattern, dLastTickLoc, -1});
          if (spBlock->m_privateBlock.m_playFlags.testFlag(ETickType::eVibrateTick))
            spBlock->m_privateBlock.m_vdSpawnedTicks.push_back(SMetronomeTick{
                ETickType::eVibrateTick, dLastTickLoc, 75});
          if (spBlock->m_privateBlock.m_playFlags.testFlag(ETickType::eLinearTick))
            spBlock->m_privateBlock.m_vdSpawnedTicks.push_back(SMetronomeTick{
                ETickType::eLinearTick, dLastTickLoc, iDistEncoded});
          if (spBlock->m_privateBlock.m_playFlags.testFlag(ETickType::eRotateTick))
            spBlock->m_privateBlock.m_vdSpawnedTicks.push_back(SMetronomeTick{
                ETickType::eRotateTick, dLastTickLoc, iLastRotateData});

          spBlock->m_privateBlock.m_lastTickHigh = !spBlock->m_privateBlock.m_lastTickHigh;

          dLastPatternTick = dLastTickLoc;
          bFoundLastPatternTick = true;
        }
      }

      if (c_iUpdateThresholdInterval < m_iCumulativeTime)
      {
        emit SignalPatternChanged(id, TransformTicksForUi(spBlock->m_privateBlock.m_vdSpawnedTicks));
      }
    }
  }

  if (c_iUpdateThresholdInterval < m_iCumulativeTime)
  {
    m_iCumulativeTime = 0.0;
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotMutedChanged()
{
  if (nullptr != m_spSettings)
  {
    m_bMuted = m_spSettings->Muted();
    for (const auto& [_, spBlock] : m_metronomeBlocks)
    {
      spBlock->m_privateBlock.m_spSoundEmitters->SetMuted(
          m_bMuted || spBlock->m_privateBlock.m_bMuted);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeManager::SlotVolumeChanged()
{
  if (nullptr != m_spSettings)
  {
    m_dVolume = m_spSettings->Volume() * m_spSettings->MetronomeVolume();
    for (const auto& [_, spBlock] : m_metronomeBlocks)
    {
      spBlock->m_privateBlock.m_spSoundEmitters->SetVolume(
          m_dVolume * spBlock->m_privateBlock.m_dVolume);
    }
  }
}
