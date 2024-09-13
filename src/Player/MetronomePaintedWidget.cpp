#include "MetronomePaintedWidget.h"
#include "Application.h"
#include "Settings.h"

#include "Systems/MetronomeManager.h"

#include <QFileInfo>
#include <QPainter>

namespace  {
  const double c_dTickWidth = 5.0;
}

CMetronomeCanvasQml::CMetronomeCanvasQml(QQuickItem* pParent) :
  QQuickPaintedItem(pParent),
  m_spMetronomeManager(CApplication::Instance()->System<CMetronomeManager>().lock()),
  m_id(QUuid::createUuid()),
  m_tickColor(Qt::white)
{
  if (nullptr != m_spMetronomeManager)
  {
    m_spDataBlockThread = m_spMetronomeManager->RegisterUi(m_id);
    connect(m_spMetronomeManager.get(), &CMetronomeManager::SignalStarted,
            this, [this](QUuid id){
      if (m_id == id)
      {
        emit metronomeStateChanged(MetronomeStateChange::Started);
      }
    }, Qt::QueuedConnection);
    connect(m_spMetronomeManager.get(), &CMetronomeManager::SignalPaused,
            this, [this](QUuid id){
      if (m_id == id)
      {
        emit metronomeStateChanged(MetronomeStateChange::Paused);
      }
    }, Qt::QueuedConnection);
    connect(m_spMetronomeManager.get(), &CMetronomeManager::SignalResumed,
            this, [this](QUuid id){
      if (m_id == id)
      {
        emit metronomeStateChanged(MetronomeStateChange::Resumed);
      }
    }, Qt::QueuedConnection);
    connect(m_spMetronomeManager.get(), &CMetronomeManager::SignalStopped,
            this, [this](QUuid id){
      if (m_id == id)
      {
        emit metronomeStateChanged(MetronomeStateChange::Stopped);
      }
    }, Qt::QueuedConnection);

    UpdatePattern();

    connect(m_spMetronomeManager.get(), &CMetronomeManager::SignalPatternChanged,
            this, &CMetronomeCanvasQml::SlotUpdate, Qt::DirectConnection);
    connect(m_spMetronomeManager.get(), &CMetronomeManager::SignalTickReachedCenter,
            this, &CMetronomeCanvasQml::SlotTickReachedCenter, Qt::QueuedConnection);
  }
}

CMetronomeCanvasQml::~CMetronomeCanvasQml()
{
  if (nullptr != m_spMetronomeManager)
  {
    m_spMetronomeManager->DeregisterUi(m_id);
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::paint(QPainter* pPainter)
{
  pPainter->save();
  pPainter->setPen(m_tickColor);

  {
    QMutexLocker locker(&m_localDataMutex);
    for (const auto& it : m_vdTickmap)
    {
      double dLeftPosition = 0 + width() / 2 * it - c_dTickWidth / 2;
      pPainter->drawRoundedRect(QRectF(dLeftPosition, 0, c_dTickWidth, height()),
                                c_dTickWidth / 2, c_dTickWidth / 2, Qt::AbsoluteSize);

      double dRightPosition = width() - width() / 2 * it + c_dTickWidth / 2;
      pPainter->drawRoundedRect(QRectF(dRightPosition, 0, c_dTickWidth, height()),
                                c_dTickWidth / 2, c_dTickWidth / 2, Qt::AbsoluteSize);
    }
  }
  pPainter->restore();
}

//----------------------------------------------------------------------------------------
//
QStringList CMetronomeCanvasQml::BeatResources() const
{
  if (nullptr != m_spDataBlockThread)
  {
    QMutexLocker l(&m_spDataBlockThread->m_mutex);
    return m_spDataBlockThread->m_sBeatResources;
  }
  return QStringList();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetBeatResources(const QStringList& sResource)
{
  QStringList sOldResource;
  if (nullptr != m_spDataBlockThread)
  {
    QMutexLocker l(&m_spDataBlockThread->m_mutex);
    sOldResource = m_spDataBlockThread->m_sBeatResources;
    m_spDataBlockThread->m_sBeatResources = sResource;
  }

  if (sOldResource != sResource && nullptr != m_spMetronomeManager)
  {
    emit m_spMetronomeManager->SignalBlockChanged(m_id);
    emit beatResourcesChanged();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CMetronomeCanvasQml::Bpm() const
{
  return m_iBpm;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetBpm(qint32 iValue)
{
  if (m_iBpm != iValue)
  {
    m_iBpm = iValue;

    UpdatePattern();
    emit bpmChanged();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CMetronomeCanvasQml::MetCmdMode() const
{
  return m_metCmdMode;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetMetCmdMode(qint32 iValue)
{
  if (static_cast<qint32>(m_metCmdMode) != iValue)
  {
    m_metCmdMode = EToyMetronomeCommandModeFlags(iValue);
    emit metCmdModeChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CMetronomeCanvasQml::Muted() const
{
  if (nullptr != m_spDataBlockThread)
  {
    QMutexLocker l(&m_spDataBlockThread->m_mutex);
    return m_spDataBlockThread->m_bMuted;
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetMuted(bool bMuted)
{
  bool bOldValue = false;
  if (nullptr != m_spDataBlockThread)
  {
    QMutexLocker l(&m_spDataBlockThread->m_mutex);
    bOldValue = m_spDataBlockThread->m_bMuted;
    m_spDataBlockThread->m_bMuted = bMuted;
  }

  if (bOldValue != bMuted && nullptr != m_spMetronomeManager)
  {
    emit m_spMetronomeManager->SignalBlockChanged(m_id);
    emit mutedChanged();
  }
}

//----------------------------------------------------------------------------------------
//
QList<double> CMetronomeCanvasQml::Pattern() const
{
  return m_vdPattern;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetPattern(const QList<double>& vdVals)
{
  if (m_vdPattern != vdVals)
  {
    m_vdPattern = vdVals;

    UpdatePattern();
    emit patternChanged();
  }
}

//----------------------------------------------------------------------------------------
//
const QColor& CMetronomeCanvasQml::TickColor() const
{
  return m_tickColor;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetTickColor(const QColor& color)
{
  if (m_tickColor != color)
  {
    m_tickColor = color;
    emit tickColorChanged();
  }
}

//----------------------------------------------------------------------------------------
//
double CMetronomeCanvasQml::Volume() const
{
  if (nullptr != m_spDataBlockThread)
  {
    QMutexLocker l(&m_spDataBlockThread->m_mutex);
    return m_spDataBlockThread->m_dVolume;
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetVolume(double dValue)
{
  double dOldValue = false;
  if (nullptr != m_spDataBlockThread)
  {
    QMutexLocker l(&m_spDataBlockThread->m_mutex);
    dOldValue = m_spDataBlockThread->m_dVolume;
    m_spDataBlockThread->m_dVolume = dValue;
  }

  if (!qFuzzyCompare(dOldValue, dValue) && nullptr != m_spMetronomeManager)
  {
    emit m_spMetronomeManager->SignalBlockChanged(m_id);
    emit volumeChanged();
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::pause()
{
  if (nullptr != m_spMetronomeManager)
  {
    emit m_spMetronomeManager->SignalPause(m_id);
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::resume()
{
  if (nullptr != m_spMetronomeManager)
  {
    emit m_spMetronomeManager->SignalResume(m_id);
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::start()
{
  if (nullptr != m_spMetronomeManager)
  {
    ETickTypeFlags flags = ETickType::ePattern;
    if (m_metCmdMode.testFlag(EToyMetronomeCommandModeFlag::eVibrate)) flags |= ETickType::eVibrateTick;
    if (m_metCmdMode.testFlag(EToyMetronomeCommandModeFlag::eLinear)) flags |= ETickType::eLinearTick;
    if (m_metCmdMode.testFlag(EToyMetronomeCommandModeFlag::eRotate)) flags |= ETickType::eRotateTick;
    emit m_spMetronomeManager->SignalStart(m_id, flags);
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::stop()
{
  if (nullptr != m_spMetronomeManager)
  {
    m_vdTickmap.clear();
    emit m_spMetronomeManager->SignalStop(m_id);
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::registerUi(const QString& sUserName)
{
  if (nullptr != m_spDataBlockThread && nullptr != m_spMetronomeManager)
  {
    QMutexLocker locker(&m_spDataBlockThread->m_mutex);
    m_spDataBlockThread->sUserName = sUserName;
    emit m_spMetronomeManager->SignalBlockChanged(m_id);
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SlotTickReachedCenter(const QUuid& id)
{
  if (id != m_id) { return; }
  emit tickReachedCenter();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SlotUpdate(const QUuid& id, const std::vector<double>& vdTicks)
{
  if (id != m_id) { return; }

  auto calltime = std::chrono::high_resolution_clock::now();

  // update is direct so we don't lose thread switching time
  // we will correct thread switch time with the internal clock
  {
    QMutexLocker l(&m_localDataMutex);
    m_lastUpdate = calltime;
  }

  bool bOk = QMetaObject::invokeMethod(this, "SlotUpdateImpl",
                                       Q_ARG(const std::vector<double>&, vdTicks));
  assert(bOk); Q_UNUSED(bOk)
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SlotUpdateImpl(const std::vector<double>& vdTicks)
{
  // calculate thread switch time with the internal clock
  qint64 iCorrectionMs = 0;
  {
    QMutexLocker l(&m_localDataMutex);
    auto now = std::chrono::high_resolution_clock::now();
    auto diffMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdate);
    iCorrectionMs = diffMs.count();
  }

  // 1s to reach center, correct with difference since last Update from the thread
  m_vdTickmap.clear();
  for (const double& dVal : vdTicks)
  {
    // fix thread switch error
    const double dModified = dVal + (static_cast<double>(iCorrectionMs) / 1'000.0);
    if (1.0 >= dModified && 0.0 <= dModified)
    {
      m_vdTickmap.push_back(dModified);
    }
  }

  QQuickPaintedItem::update();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::UpdatePattern()
{
  if (nullptr != m_spDataBlockThread && nullptr != m_spMetronomeManager)
  {
    QMutexLocker locker(&m_spDataBlockThread->m_mutex);
    m_spDataBlockThread->m_vdTickPattern.clear();
    double dDefaultIntervalMs = 60'000.0 / double(m_iBpm);
    for (double dValue : m_vdPattern)
    {
      m_spDataBlockThread->m_vdTickPattern.push_back(dValue*dDefaultIntervalMs);
    }
    emit m_spMetronomeManager->SignalBlockChanged(m_id);
  }
}
