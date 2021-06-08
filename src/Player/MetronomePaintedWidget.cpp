#include "MetronomePaintedWidget.h"
#include "Systems/Resource.h"
#include <QtAV>
#include <QFileInfo>
#include <QPainter>

namespace  {
  const double c_dTickWidth = 5.0;
  const qint32 c_iNumAutioEmitters = 5;
}

CMetronomeCanvasQml::CMetronomeCanvasQml(QQuickItem* pParent) :
  QQuickPaintedItem(pParent),
  m_vspPlayers(),
  m_sBeatResource(":/resources/sound/metronome_default.wav"),
  m_tickColor(Qt::white),
  m_vdTickmap(),
  m_iLastAutioPlayer(0),
  m_bMuted(false)
{
  for (qint32 i = 0; c_iNumAutioEmitters > i; ++i)
  {
    m_vspPlayers.push_back(std::make_unique<QtAV::AVPlayer>());
    m_vspPlayers.back()->setRepeat(0);
    m_vspPlayers.back()->setFile(m_sBeatResource);
  }
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
  return m_sBeatResource;
}

//----------------------------------------------------------------------------------------
//
void CMetronomeCanvasQml::SetBeatResource(const QString& sResource)
{
  if (m_sBeatResource != sResource && (sResource.isEmpty() || QFileInfo(m_sBeatResource).exists()))
  {
    if (sResource.isEmpty())
    {
      m_sBeatResource = ":/resources/sound/metronome_default.wav";
    }
    else
    {
      m_sBeatResource = sResource;
    }
    for (qint32 i = 0; static_cast<qint32>(m_vspPlayers.size()) > i; ++i)
    {
      m_vspPlayers[i]->setFile(m_sBeatResource);
    }
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
    if (m_bMuted)
    {
      for (qint32 i = 0; static_cast<qint32>(m_vspPlayers.size()) > i; ++i)
      {
        if (m_vspPlayers[i]->isPlaying())
        {
          m_vspPlayers[i]->stop();
        }
      }
    }
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
void CMetronomeCanvasQml::clear()
{
  m_vdTickmap.clear();
  for (qint32 i = 0; static_cast<qint32>(m_vspPlayers.size()) > i; ++i)
  {
    if (m_vspPlayers[i]->isPlaying())
    {
      m_vspPlayers[i]->stop();
    }
  }
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

      if (!m_bMuted)
      {
        // play sound
        if (-1 < m_iLastAutioPlayer && static_cast<qint32>(m_vspPlayers.size()) > m_iLastAutioPlayer)
        {
          if (m_vspPlayers[m_iLastAutioPlayer]->isPlaying())
          {
            m_vspPlayers[m_iLastAutioPlayer]->stop();
          }
          m_vspPlayers[m_iLastAutioPlayer]->play();
        }

        // next audio player
        if (static_cast<qint32>(m_vspPlayers.size()) <= ++m_iLastAutioPlayer)
        {
          m_iLastAutioPlayer = 0;
        }
      }

      emit tickReachedCenter();
    }
    else
    {
      ++it;
    }
  }

  QQuickPaintedItem::update();
}
