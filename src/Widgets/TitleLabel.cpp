#include "TitleLabel.h"
#include "Application.h"
#include "Constants.h"

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QProxyStyle>

namespace  {
  const static qint32 c_iKernelSize = 19;
  const double c_iOffsetBorder = 5.0;

  //--------------------------------------------------------------------------------------
  //
  void DilationBorder(const QImage& in, QImage& out, qint32 iKernel, QRgb color)
  {
    out = in.copy();
    const QRect& dims = in.rect();
    for(qint32 x = dims.x(); x < dims.width(); x++)
    {
      for(qint32 y = dims.y(); y < dims.height(); y++)
      {
        QRgb pixel = 0;
        for (qint32 i = x - iKernel / 2; i < x + iKernel / 2 + 1; i++)
        {
          for (qint32 j = y - iKernel / 2; j < y + iKernel / 2 + 1; j++)
          {
            if (std::sqrt(std::abs(x-i) * std::abs(x-i) + std::abs(y-j) * std::abs(y-j)) <= iKernel / 2)
            {
              if (dims.x() < i && dims.width() > i &&
                  dims.y() < j && dims.height() > j)
              {
                if (pixel < in.pixel(i, j))
                {
                  pixel = color;
                  goto endKernel;
                }
              }
            }
          }
        }
        endKernel:
        out.setPixel(x, y, pixel);
      }  /* ends loop over j */
    }  /* ends loop over i */
  }

  //--------------------------------------------------------------------------------------
  //
  void Gauss15x15(const QImage& in, QImage& out)
  {
    static const qint32 iHalfKernelSize = 7;
    static const double kernel[] = {
      0, 0, 0, 0, 0, 0.000001, 0.000001, 0.000002, 0.000001, 0.000001, 0, 0, 0, 0, 0,
      0, 0, 0, 0.000001, 0.000004, 0.000013, 0.000024, 0.00003, 0.000024, 0.000013, 0.000004, 0.000001, 0, 0, 0,
      0, 0, 0.000002, 0.00001, 0.000047, 0.000136, 0.000259, 0.00032, 0.000259, 0.000136, 0.000047, 0.00001, 0.000002, 0, 0,
      0, 0.000001, 0.00001, 0.000072, 0.000321, 0.000938, 0.001784, 0.00221, 0.001784, 0.000938, 0.000321, 0.000072, 0.00001, 0.000001, 0,
      0, 0.000004, 0.000047, 0.000321, 0.001442, 0.00421, 0.008005, 0.009916, 0.008005, 0.00421, 0.001442, 0.000321, 0.000047, 0.000004, 0,
      0.000001, 0.000013, 0.000136, 0.000938, 0.00421, 0.012291, 0.023369, 0.028949, 0.023369, 0.012291, 0.00421, 0.000938, 0.000136, 0.000013, 0.000001,
      0.000001, 0.000024, 0.000259, 0.001784, 0.008005, 0.023369, 0.044431, 0.05504, 0.044431, 0.023369, 0.008005, 0.001784, 0.000259, 0.000024, 0.000001,
      0.000002, 0.00003, 0.00032, 0.00221, 0.009916, 0.028949, 0.05504, 0.068182, 0.05504, 0.028949, 0.009916, 0.00221, 0.00032, 0.00003, 0.000002,
      0.000001, 0.000024, 0.000259, 0.001784, 0.008005, 0.023369, 0.044431, 0.05504, 0.044431, 0.023369, 0.008005, 0.001784, 0.000259, 0.000024, 0.000001,
      0.000001, 0.000013, 0.000136, 0.000938, 0.00421, 0.012291, 0.023369, 0.028949, 0.023369, 0.012291, 0.00421, 0.000938, 0.000136, 0.000013, 0.000001,
      0, 0.000004, 0.000047, 0.000321, 0.001442, 0.00421, 0.008005, 0.009916, 0.008005, 0.00421, 0.001442, 0.000321, 0.000047, 0.000004, 0,
      0, 0.000001, 0.00001, 0.000072, 0.000321, 0.000938, 0.001784, 0.00221, 0.001784, 0.000938, 0.000321, 0.000072, 0.00001, 0.000001, 0,
      0, 0, 0.000002, 0.00001, 0.000047, 0.000136, 0.000259, 0.00032, 0.000259, 0.000136, 0.000047, 0.00001, 0.000002, 0, 0,
      0, 0, 0, 0.000001, 0.000004, 0.000013, 0.000024, 0.00003, 0.000024, 0.000013, 0.000004, 0.000001, 0, 0, 0,
      0, 0, 0, 0, 0, 0.000001, 0.000001, 0.000002, 0.000001, 0.000001, 0, 0, 0, 0, 0
    };

    out = QImage(in.size(), in.format());
    out.fill(0);
    const QRect& dims = in.rect();
    for(qint32 x = dims.x(); x < dims.width(); x++)
    {
      for(qint32 y = dims.y(); y < dims.height(); y++)
      {
        QRgb pixel = 0;
        qint32 iKernelIndex = 0;
        for (qint32 i = x - iHalfKernelSize; i < x + iHalfKernelSize + 1; i++)
        {
          for (qint32 j = y - iHalfKernelSize; j < y + iHalfKernelSize + 1; j++)
          {
            QRgb currentPixels = (dims.x() < i && dims.width() > i &&
                dims.y() < j && dims.height() > j) ? in.pixel(i, j) : 0;
            pixel += qRgba(
                static_cast<qint32>(static_cast<double>(qRed(currentPixels)) * kernel[iKernelIndex]),
                static_cast<qint32>(static_cast<double>(qGreen(currentPixels)) * kernel[iKernelIndex]),
                static_cast<qint32>(static_cast<double>(qBlue(currentPixels)) * kernel[iKernelIndex]),
                static_cast<qint32>(static_cast<double>(qAlpha(currentPixels)) * kernel[iKernelIndex]));
            iKernelIndex++;
          }
        }
        out.setPixel(x, y, pixel);
      }  /* ends loop over j */
    }  /* ends loop over i */
  }
}

//----------------------------------------------------------------------------------------
//
class CTitleProxyStyle : public QProxyStyle
{
public:
  CTitleProxyStyle() :
    QProxyStyle(),
    m_bDirty(true),
    m_backgroundImage(),
    m_outlineColor(Qt::white)
  {}

  //--------------------------------------------------------------------------------------
  //
  void SetDirty() { m_bDirty = true; }

  //--------------------------------------------------------------------------------------
  //
  void SetOutlineColor(const QColor& color)
  {
    if (m_outlineColor != color)
    {
      m_outlineColor = color;
      m_bDirty = true;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  const QColor& OutlineColor()
  {
    return m_outlineColor;
  }

protected:
  //--------------------------------------------------------------------------------------
  //
  virtual void drawItemText(QPainter* pPainter, const QRect &rect,
      int flags, const QPalette &pal, bool enabled,
      const QString &text, QPalette::ColorRole textRole) const
  {
    // draw background to offscreen buffer
    if (m_bDirty)
    {
      // hack to un-const cast the pointer, since we want to cache the render
      // this prevents unnessesarily long draw calls
      CTitleProxyStyle* ptr = const_cast<CTitleProxyStyle*>(this);

      QImage offScreenBuffer(rect.size(), QImage::Format_ARGB32);
      offScreenBuffer.fill(Qt::transparent);

      QPainter offscreenPainter(&offScreenBuffer);
      offscreenPainter.initFrom(pPainter->device());
      offscreenPainter.setPen(m_outlineColor);
      offscreenPainter.setBrush(pal.text());
      offscreenPainter.drawText(rect, flags, text);

      // process background
      QImage dilatedPixmap;
      DilationBorder(offScreenBuffer, dilatedPixmap, c_iKernelSize, m_outlineColor.rgba());
      // debug
      //dilatedPixmap.save("bla.png");
      Gauss15x15(dilatedPixmap, ptr->m_backgroundImage);
      // debug
      //gaussFiltered.save("bla.png");

      ptr->m_bDirty = false;
    }

    // draw background
    pPainter->save();
    pPainter->setPen(m_outlineColor);
    pPainter->setBrush(m_outlineColor);
    pPainter->drawImage(m_backgroundImage.rect(), m_backgroundImage, m_backgroundImage.rect());
    pPainter->restore();

    // draw shadow
    pPainter->save();
    for (double i = 0.0; i < c_iOffsetBorder; i += 1)
    {
      QRect translatedRect =
          rect.translated(static_cast<qint32>(i), static_cast<qint32>(i));
      QColor shadowColor(pal.text().color());
      pPainter->setPen(shadowColor.darker(200));
      pPainter->setBrush(pal.text());
      pPainter->drawText(translatedRect, flags, text);
    }
    pPainter->restore();

    // draw text
    QProxyStyle::drawItemText(pPainter, rect, flags, pal, enabled, text, textRole);
  }

  bool   m_bDirty;
  QImage m_backgroundImage;
  QColor m_outlineColor;
};

//----------------------------------------------------------------------------------------
//
CTitleLabel::CTitleLabel(QWidget* pParent) :
  QLabel(pParent)
{
  connect(CApplication::Instance(), &CApplication::StyleLoaded,
          this, &CTitleLabel::SlotStyleLoaded, Qt::QueuedConnection);

  QFont thisFont = font();
  thisFont.setPixelSize(60);
  setFont(thisFont);
  m_pStyle = new CTitleProxyStyle();
  m_pStyle->setParent(this);
  setStyle(m_pStyle);
  QFontMetrics fontMetrics(thisFont);
  setFixedHeight(fontMetrics.height() + c_iKernelSize + static_cast<qint32>(c_iOffsetBorder));
  AddEffects();
}

CTitleLabel::CTitleLabel(QString sText, QWidget* pParent) :
  QLabel(sText, pParent)
{
  connect(CApplication::Instance(), &CApplication::StyleLoaded,
          this, &CTitleLabel::SlotStyleLoaded, Qt::QueuedConnection);

  QFont thisFont = font();
  thisFont.setPixelSize(60);
  setFont(thisFont);
  m_pStyle = new CTitleProxyStyle();
  m_pStyle->setParent(this);
  setStyle(m_pStyle);
  QFontMetrics fontMetrics(thisFont);
  setFixedHeight(fontMetrics.height() + c_iKernelSize + static_cast<qint32>(c_iOffsetBorder));
  AddEffects();
}

//----------------------------------------------------------------------------------------
//
void CTitleLabel::SetOutlineColor(const QColor& color)
{
  m_pStyle->SetOutlineColor(color);
}

//----------------------------------------------------------------------------------------
//
const QColor& CTitleLabel::OutlineColor()
{
  return m_pStyle->OutlineColor();
}

//----------------------------------------------------------------------------------------
//
void CTitleLabel::SlotStyleLoaded()
{
  QFont thisFont = font();
  thisFont.setPixelSize(60);
  thisFont.setFamily(CApplication::Instance()->Settings()->Font());
  setFont(thisFont);
  m_pStyle->SetDirty();
}

//----------------------------------------------------------------------------------------
//
void CTitleLabel::AddEffects()
{
  QGraphicsDropShadowEffect* pShadow = new QGraphicsDropShadowEffect(this);
  pShadow->setBlurRadius(10);
  pShadow->setXOffset(5);
  pShadow->setYOffset(5);
  pShadow->setColor(Qt::black);
  setGraphicsEffect(pShadow);
}
