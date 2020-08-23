#include "MetronomePaintedWidget.h"
#include "Systems/Resource.h"
#include <QtAV>
#include <QFileInfo>
#include <QPainter>

namespace  {
  const double c_dTickWidth = 5.0;
}

CMetronomeCanvasQml::CMetronomeCanvasQml(QQuickItem* pParent) :
  QQuickPaintedItem(pParent),
  m_player(std::make_unique<QtAV::AVPlayer>()),
  m_sBeatResource(":/resources/sound/metronome_default.wav"),
  m_tickColor(Qt::white),
  m_vdTickmap()
{
  m_player->setRepeat(0);
  m_player->setFile(m_sBeatResource);
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
  if (m_sBeatResource != sResource && QFileInfo(m_sBeatResource).exists())
  {
    if (sResource.isEmpty())
    {
      m_sBeatResource = ":/resources/sound/metronome_default.wav";
    }
    else
    {
      m_sBeatResource = sResource;
    }
    m_player->setFile(m_sBeatResource);
    emit beatResourceChanged();
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

      // play sound
      if (m_player->isPlaying())
      {
        m_player->stop();
      }
      m_player->play();

      emit tickReachedCenter();
    }
    else
    {
      ++it;
    }
  }

  QQuickPaintedItem::update();
}
