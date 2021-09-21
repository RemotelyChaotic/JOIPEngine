#include "TitleLabel.h"
#include "Application.h"
#include "Constants.h"

#include <QFont>
#include <QFutureWatcher>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPropertyAnimation>
#include <QProxyStyle>
#include <QtConcurrent/QtConcurrent>

namespace  {
  const qint32 c_iKernelSize = 19;
  const qint32 c_iHalfKernelSize = 7;
  const double c_iOffsetBorder = 5.0;
  const qint32 c_iAnimationTime = 1500;
  const qint32 c_iUpdateInterval = 5;

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
            if (std::abs(x-i) * std::abs(x-i) + std::abs(y-j) * std::abs(y-j) <= iKernel*iKernel / 4)
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
        for (qint32 i = x - c_iHalfKernelSize; i < x + c_iHalfKernelSize + 1; i++)
        {
          for (qint32 j = y - c_iHalfKernelSize; j < y + c_iHalfKernelSize + 1; j++)
          {
            QRgb currentPixels = (dims.x() < i && dims.width() > i &&
                dims.y() < j && dims.height() > j) ? in.pixel(i, j) : 0;
            pixel += qRgba(
                static_cast<qint32>(kernel[iKernelIndex] * qRed(currentPixels)),
                static_cast<qint32>(kernel[iKernelIndex] * qGreen(currentPixels)),
                static_cast<qint32>(kernel[iKernelIndex] * qBlue(currentPixels)),
                static_cast<qint32>(kernel[iKernelIndex] * qAlpha(currentPixels)));
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
  Q_OBJECT
  Q_PROPERTY(double progress MEMBER m_dProgress)
  Q_PROPERTY(double progress2 MEMBER m_dProgress2)

public:
  CTitleProxyStyle() :
    QProxyStyle(),
    m_spFutureWatcher(std::make_unique<QFutureWatcher<void>>()),
    m_bRendering(false),
    m_bDirty(true),
    m_backgroundImage(),
    m_outlineColor(Qt::white),
    m_dProgress(0.0)
  {
    m_pProgressAnimation = new QPropertyAnimation(this, "progress");
    m_pProgressAnimation->setDuration(c_iAnimationTime);
    m_pProgressAnimation->setEasingCurve(QEasingCurve::InQuad);
    m_pProgressAnimation->setLoopCount(1);

    m_pProgress2Animation = new QPropertyAnimation(this, "progress2");
    m_pProgress2Animation->setDuration(c_iAnimationTime);
    m_pProgress2Animation->setEasingCurve(QEasingCurve::InQuad);
    m_pProgress2Animation->setLoopCount(1);
    connect(m_pProgress2Animation, &QPropertyAnimation::finished,
            this, &CTitleProxyStyle::AnimationsFinished);
  }
  ~CTitleProxyStyle()
  {
    m_future.cancel();
    m_future.waitForFinished();
  }

  //--------------------------------------------------------------------------------------
  //
  void SetDirty()
  {
    m_pProgressAnimation->stop();
    m_pProgress2Animation->stop();

    if (m_spFutureWatcher->isRunning())
    {
      m_spFutureWatcher->cancel();
      m_spFutureWatcher->waitForFinished();
    }

    m_bDirty = true;
    m_bRendering = false;
    m_dProgress = 0.0;
    m_dProgress2 = 0.0;

    m_pProgressAnimation->setStartValue(m_dProgress);
    m_pProgressAnimation->setEndValue(1.0);
    m_pProgressAnimation->start();

    m_pProgress2Animation->setStartValue(m_dProgress2);
    m_pProgress2Animation->setEndValue(1.0);
  }

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

signals:
  void AnimationsFinished();
  void LoadingFinished();

protected slots:
  void SlotImageLoad(const QBrush &textBrush, const QRect& /*rect*/,
                     int /*flags*/, const QString& sText, const QFont& font)
  {
    QFontMetrics info(font);
    QRect rectBuffer = info.boundingRect(sText)
                            .adjusted(-c_iKernelSize+1, -c_iKernelSize+1,
                                      c_iKernelSize+1, c_iKernelSize+1).normalized();
    rectBuffer = QRect({0, 0}, rectBuffer.size());

    QImage offScreenBuffer(rectBuffer.size(), QImage::Format_ARGB32);
    offScreenBuffer.fill(Qt::transparent);

    QPainter offscreenPainter(&offScreenBuffer);
    offscreenPainter.setPen(m_outlineColor);
    offscreenPainter.setBrush(textBrush);
    offscreenPainter.setFont(font);
    offscreenPainter.drawText(rectBuffer, Qt::AlignCenter, sText);

    // process background
    QImage dilatedPixmap;
    DilationBorder(offScreenBuffer, dilatedPixmap, c_iKernelSize, m_outlineColor.rgba());
    // debug
    //dilatedPixmap.save("bla.png");
    Gauss15x15(dilatedPixmap, m_backgroundImage);
    // debug
    //m_backgroundImage.save("bla.png");

    m_bDirty = false;
    m_bRendering = false;
    emit LoadingFinished();
    QMetaObject::invokeMethod(m_pProgress2Animation, "start", Qt::QueuedConnection);
  }

protected:
  void StartLoad(const QBrush &textBrush, const QRect &rect,
                 int flags, const QString& sText, const QFont& font)
  {
    m_bRendering = true;
    m_future = QtConcurrent::run(this, &CTitleProxyStyle::SlotImageLoad,
                                 textBrush, rect, flags, sText, font);
    m_spFutureWatcher->setFuture(m_future);
  }

  //--------------------------------------------------------------------------------------
  //
  virtual void drawItemText(QPainter* pPainter, const QRect &rect,
      int flags, const QPalette &pal, bool /*enabled*/,
      const QString &text, QPalette::ColorRole /*textRole*/) const
  {
    // draw background to offscreen buffer
    if (m_bDirty)
    {
      if (!m_bRendering)
      {
        // hack to un-const cast the pointer, since we want to cache the render
        // this prevents unnessesarily long draw calls
        CTitleProxyStyle* ptr = const_cast<CTitleProxyStyle*>(this);
        ptr->StartLoad(pal.text(), rect, flags, text, pPainter->font());
      }
    }
    else
    {
      // draw background
      pPainter->save();
      pPainter->setOpacity(m_dProgress);
      pPainter->setPen(m_outlineColor);
      pPainter->setBrush(m_outlineColor);
      QSize sizeDiff(rect.size() - m_backgroundImage.rect().size());
      pPainter->drawImage(
            m_backgroundImage.rect().translated(sizeDiff.width()/2, sizeDiff.height()/2),
            m_backgroundImage, m_backgroundImage.rect());
      pPainter->restore();
    }

    // draw shadow
    pPainter->save();
    for (double i = 0.0; i < c_iOffsetBorder; i += 1)
    {
      QRect translatedRect =
          rect.translated(static_cast<qint32>(i), static_cast<qint32>(i));
      QColor shadowColor(pal.text().color());
      shadowColor.setAlpha(m_dProgress*255);
      pPainter->setPen(shadowColor.darker(200));
      pPainter->setBrush(pal.text());
      pPainter->drawText(translatedRect, flags, text);
    }
    pPainter->restore();

    // draw text
    QColor col = pal.text().color();
    col.setAlpha(m_dProgress*255);
    pPainter->setPen(col);
    pPainter->setBrush(pal.text());
    pPainter->drawText(rect, flags, text);
  }

  std::unique_ptr<QFutureWatcher<void>> m_spFutureWatcher;
  QFuture<void>                         m_future;
  QAtomicInt                            m_bRendering;
  QAtomicInt                            m_bDirty;
  QImage                                m_backgroundImage;
  QColor                                m_outlineColor;
  QPointer<QPropertyAnimation>          m_pProgressAnimation;
  double                                m_dProgress;
  QPointer<QPropertyAnimation>          m_pProgress2Animation;
  double                                m_dProgress2;
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
  connect(m_pStyle, &CTitleProxyStyle::LoadingFinished,
          this, &CTitleLabel::SlotUpdate, Qt::QueuedConnection);
  connect(m_pStyle, &CTitleProxyStyle::AnimationsFinished,
          this, [this](){ SlotUpdate(); m_updateTimer.stop(); });

  QFontMetrics fontMetrics(thisFont);
  setFixedHeight(fontMetrics.height() + c_iKernelSize + static_cast<qint32>(c_iOffsetBorder));
  AddEffects();

  m_updateTimer.setInterval(c_iUpdateInterval);
  m_updateTimer.setSingleShot(false);
  connect(&m_updateTimer, &QTimer::timeout,
          this, &CTitleLabel::SlotUpdate, Qt::DirectConnection);
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
  connect(m_pStyle, &CTitleProxyStyle::LoadingFinished,
          this, &CTitleLabel::SlotUpdate, Qt::QueuedConnection);
  connect(m_pStyle, &CTitleProxyStyle::AnimationsFinished,
          this, [this](){ SlotUpdate(); m_updateTimer.stop(); });

  QFontMetrics fontMetrics(thisFont);
  setFixedHeight(fontMetrics.height() + c_iKernelSize + static_cast<qint32>(c_iOffsetBorder));
  AddEffects();

  m_updateTimer.setInterval(c_iUpdateInterval);
  m_updateTimer.setSingleShot(false);
  connect(&m_updateTimer, &QTimer::timeout,
          this, &CTitleLabel::SlotUpdate, Qt::DirectConnection);
  m_updateTimer.start();
}

//----------------------------------------------------------------------------------------
//
void CTitleLabel::SetOutlineColor(const QColor& color)
{
  m_pStyle->SetOutlineColor(color);
  m_pStyle->SetDirty();
  m_updateTimer.start();
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
  m_updateTimer.start();
}

//----------------------------------------------------------------------------------------
//
void CTitleLabel::SlotUpdate()
{
  repaint();
}

//----------------------------------------------------------------------------------------
//
void CTitleLabel::resizeEvent(QResizeEvent* pEvt)
{
  Q_UNUSED(pEvt)
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

#include "TitleLabel.moc"
