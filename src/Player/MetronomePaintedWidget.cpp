#include "MetronomePaintedWidget.h"
#include "Application.h"
#include "Settings.h"
#include "Systems/Resource.h"
#include "Utils/MultiEmitterSoundPlayer.h"
#include <QFileInfo>
#include <QPainter>

namespace  {
  const double c_dTickWidth = 5.0;
}

CMetronomeCanvasQml::CMetronomeCanvasQml(QQuickItem* pParent) :
  QQuickPaintedItem(pParent),
  m_spSoundEmitters(
    std::make_unique<CMultiEmitterSoundPlayer>(CMultiEmitterSoundPlayer::c_iDefaultNumAutioEmitters,
                                               ":/resources/sound/metronome_default.wav")),
  m_spSettings(CApplication::Instance()->Settings()),
  m_sBeatResource(":/resources/sound/metronome_default.wav"),
  m_tickColor(Qt::white),
  m_vdTickmap(),
  m_dVolume(1.0)
{
  if (nullptr != m_spSettings)
  {
    connect(m_spSettings.get(), &CSettings::mutedChanged,
            this, &CMetronomeCanvasQml::SlotMutedChanged, Qt::QueuedConnection);
    connect(m_spSettings.get(), &CSettings::volumeChanged,
            this, &CMetronomeCanvasQml::SlotVolumeChanged, Qt::QueuedConnection);
  }

  SlotMutedChanged();
  SlotVolumeChanged();
}

CMetronomeCanvasQml::~CMetronomeCanvasQml()
{

}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::paint(QPainter* pPainter)
{
  pPainter->save();
  pPainter->setPen(m_tickColor);

  for (const auto& it : m_vdTickmap)
  {
    double dLeftPosition = 0 + width() / 2 * it - c_dTickWidth / 2;
    pPainter->drawRoundedRect(QRectF(dLeftPosition, 0, c_dTickWidth, height()),
                              c_dTickWidth / 2, c_dTickWidth / 2, Qt::AbsoluteSize);

    double dRightPosition = width() - width() / 2 * it + c_dTickWidth / 2;
    pPainter->drawRoundedRect(QRectF(dRightPosition, 0, c_dTickWidth, height()),
                              c_dTickWidth / 2, c_dTickWidth / 2, Qt::AbsoluteSize);
  }
  pPainter->restore();
}

//----------------------------------------------------------------------------------------
//
const QString& CMetronomeCanvasQml::BeatResource() const
{
  return m_spSoundEmitters->SoundEffect();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetBeatResource(const QString& sResource)
{
  const QString sOldResource = m_spSoundEmitters->SoundEffect();
  if (sResource.isEmpty())
  {
    m_spSoundEmitters->SetSoundEffect(":/resources/sound/metronome_default.wav");
  }
  else
  {
    m_spSoundEmitters->SetSoundEffect(sResource);
  }

  if (sOldResource != m_spSoundEmitters->SoundEffect())
  {
    emit beatResourceChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CMetronomeCanvasQml::Muted() const
{
  return m_bMuted;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetMuted(bool bMuted)
{
  if (m_bMuted != bMuted)
  {
    m_bMuted = bMuted;
    SlotMutedChanged();
    emit mutedChanged();
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
  return m_dVolume;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetVolume(double dValue)
{
  if (!qFuzzyCompare(m_dVolume, dValue))
  {
    m_dVolume = dValue;
    SlotVolumeChanged();
    emit volumeChanged();
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::clear()
{
  m_vdTickmap.clear();
  m_spSoundEmitters->Stop();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::spawnNewMetronomeTicks()
{
  m_vdTickmap.push_back(0.0);
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::update(double dIntervalMs)
{
  for (auto it = m_vdTickmap.begin(); m_vdTickmap.end() != it;)
  {
    // 1s to reach the center, tickmap is percentage based
    *it += dIntervalMs / 3000;
    if (1.0 < *it)
    {
      *it = 1.0;
      it = m_vdTickmap.erase(it);

      m_spSoundEmitters->Play();

      emit tickReachedCenter();
    }
    else
    {
      ++it;
    }
  }

  QQuickPaintedItem::update();
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SlotMutedChanged()
{
  if (nullptr != m_spSettings)
  {
    m_spSoundEmitters->SetMuted(m_bMuted || m_spSettings->Muted());
  }
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SlotVolumeChanged()
{
  if (nullptr != m_spSettings)
  {
    m_spSoundEmitters->SetVolume(m_spSettings->Volume() * m_dVolume);
  }
}
