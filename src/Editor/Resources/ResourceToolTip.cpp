#include "ResourceToolTip.h"
#include "Application.h"
#include "ResourceDetailViewFetcherThread.h"
#include "Style.h"
#include "ui_ResourceToolTip.h"

#include "Systems/Project.h"

#include <QBuffer>
#include <QDebug>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QToolTip>

#include <optional>

namespace
{
  const qint32 c_iPreviewImageSize = 48;
}

class CResourceToolTipPrivate : public QObject
{
  Q_OBJECT
public:
  CResourceToolTipPrivate();
  ~CResourceToolTipPrivate();
  static std::shared_ptr<CResourceToolTipPrivate> Instance();

  bool IsShown() const { return m_bShown; }
  tspResource Resource() const;
  std::shared_ptr<CResourceDetailViewFetcherThread> ResourceFetcher() const;
  void ShowResource(const QPoint& pos, const tspResource& spResource,
                    QWidget* pW, const QRect& rect, qint32 iMsecShowTime);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

protected slots:
  void SlotResourceLoadFinished(const QString& sResource, const QString& pixmapBase64);

private:
  struct STipData
  {
    QPoint pos;
    tspResource spResource;
    QPointer<QWidget> pW;
    QRect rect;
    qint32 iMsecShowTime;
  };

  QString GetTipString(const STipData& data, const QString& pixmapBase64);
  qint32 GetTipScreen(const QPoint& pos, QWidget* pW);
  void InstallToolTipFilter(const QPoint& pos, QWidget* pW);

  static std::shared_ptr<CResourceToolTipPrivate>        m_spInstance;
  std::unique_ptr<CThreadedSystem>                       m_spThreadedLoader;
  std::optional<STipData>                                m_currentRequest = std::nullopt;
  bool                                                   m_bShown = false;
};

//----------------------------------------------------------------------------------------
//
CResourceToolTipPrivate::CResourceToolTipPrivate() :
  m_spThreadedLoader(std::make_unique<CThreadedSystem>("ResourceToolTipFetcher"))
{
  m_spThreadedLoader->RegisterObject<CResourceDetailViewFetcherThread>();
  connect(ResourceFetcher().get(),
          qOverload<const QString&,const QString&>(&CResourceDetailViewFetcherThread::LoadFinished),
          this, &CResourceToolTipPrivate::SlotResourceLoadFinished);
}
CResourceToolTipPrivate::~CResourceToolTipPrivate()
{

}
std::shared_ptr<CResourceToolTipPrivate> CResourceToolTipPrivate::m_spInstance = nullptr;

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CResourceToolTipPrivate> CResourceToolTipPrivate::Instance()
{
  if (nullptr == m_spInstance)
  {
    m_spInstance = std::make_shared<CResourceToolTipPrivate>();
  }
  return m_spInstance;
}

//----------------------------------------------------------------------------------------
//
tspResource CResourceToolTipPrivate::Resource() const
{
  return m_currentRequest.has_value() ? m_currentRequest.value().spResource : nullptr;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CResourceDetailViewFetcherThread> CResourceToolTipPrivate::ResourceFetcher() const
{
  return std::static_pointer_cast<CResourceDetailViewFetcherThread>(m_spThreadedLoader->Get());
}

//----------------------------------------------------------------------------------------
//
void CResourceToolTipPrivate::ShowResource(const QPoint& pos, const tspResource& spResource,
                                           QWidget* pW, const QRect& rect, qint32 iMsecShowTime)
{
  if (nullptr != spResource)
  {
    m_currentRequest = STipData{
      pos, spResource, pW, rect, iMsecShowTime
    };

    if (!m_bShown)
    {
      InstallToolTipFilter(pos, pW);
    }

    qint32 iProject = -1;
    QString sName;
    EResourceType type = EResourceType::eOther;
    {
      QReadLocker resourceLocker(&spResource->m_rwLock);
      sName = spResource->m_sName;
      type = spResource->m_type;
      if (nullptr != spResource->m_spParent)
      {
        QReadLocker locker(&spResource->m_spParent->m_rwLock);
        iProject = spResource->m_spParent->m_iId;
      }
    }

    if (EResourceType::eImage == type._to_integral() ||
        EResourceType::eMovie == type._to_integral())
    {
      if (ResourceFetcher()->IsLoading())
      {
        ResourceFetcher()->AbortLoading();
      }
      ResourceFetcher()->RequestResources(iProject, QStringList() << sName,
                                          QSize(c_iPreviewImageSize, c_iPreviewImageSize));
    }
    else
    {
      SlotResourceLoadFinished(sName, QString());
    }
    m_bShown = true;
  }
  else
  {
    QToolTip::hideText();
    m_currentRequest = std::nullopt;
    m_bShown = false;
  }
}

//----------------------------------------------------------------------------------------
//
bool CResourceToolTipPrivate::eventFilter(QObject* pObj, QEvent* pEvt)
{
  if (nullptr != pObj && nullptr != pEvt)
  {
    if (pEvt->type() == QEvent::Hide)
    {
      pObj->removeEventFilter(this);
      m_currentRequest = std::nullopt;
      m_bShown = false;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CResourceToolTipPrivate::SlotResourceLoadFinished(
    const QString& sName, const QString& pixmapBase64)
{
  if (m_currentRequest.has_value())
  {
    STipData data = m_currentRequest.value();

    QReadLocker locker(&data.spResource->m_rwLock);
    if (data.spResource->m_sName == sName)
    {
      const QString sTipData = GetTipString(data, pixmapBase64);
      QToolTip::showText(data.pos, sTipData, data.pW, data.rect, data.iMsecShowTime);
    }
  }

  m_currentRequest = std::nullopt;
}

//----------------------------------------------------------------------------------------
//
QString CResourceToolTipPrivate::GetTipString(const STipData& data,
                                              const QString& pixmapBase64)
{
  QString sRet = QString();

  QFont font = CApplication::Instance()->font();
  QString sFontFace = font.family();
  qint32 iFontsize = font.pointSize();

  if (EResourceType::eMovie == data.spResource->m_type._to_integral() ||
      EResourceType::eImage == data.spResource->m_type._to_integral())
  {
    sRet =
        "<table><tr>"
          "<td style=\"text-align: center; vertical-align: middle;\">"
          "<img src=\"data:image/png;base64,%3\" alt=\"%4\" style=\"width:%5px;height:%6px;\"/>"
          "</td>"
          "<td><p style=\"font-family:'%1';font-size:%2px;\">"
          "Source: %7<br>Bundle: %8"
          "</p></td>"
        "</tr></table>";

    sRet = sRet.arg(sFontFace).arg(iFontsize)
        .arg(pixmapBase64)
        .arg(data.spResource->m_sName)
        .arg(c_iPreviewImageSize).arg(c_iPreviewImageSize);
  }
  else if (EResourceType::eSound == data.spResource->m_type._to_integral())
  {
    sRet =
      "<p style=\"font-family:'%1';font-size:%2px\">"
        "Source: %3<br>Bundle: %4"
      "</p>";

    sRet = sRet.arg(sFontFace).arg(iFontsize);
  }
  else if (EResourceType::eScript == data.spResource->m_type._to_integral())
  {
    sRet =
      "<p style=\"font-family:'%1';font-size:%2px\">"
        "Type: %3<br>Source: %4<br>Bundle: %5"
      "</p>";

    sRet = sRet.arg(sFontFace).arg(iFontsize)
        .arg(QFileInfo(data.spResource->m_sPath.toString()).suffix());
  }
  else if (EResourceType::eFont == data.spResource->m_type._to_integral())
  {
    sRet =
      "<p style=\"font-family:'%1';font-size:%2px;\">"
        "Font-Family: <span style=\"font-family:'%3';font-size:%4px;\">%5</span><br>Source: %6<br>Bundle: %7"
      "</p>";
    const QStringList vsFamilies =
        QFontDatabase::applicationFontFamilies(data.spResource->m_iLoadedId);
    if (vsFamilies.size() > 0)
    {
      QString sFontFace2 = vsFamilies.first();
      sRet = sRet.arg(sFontFace).arg(iFontsize).arg(sFontFace2).arg(iFontsize).arg(sFontFace2);
    }
    else
    {
      sRet = sRet.arg(sFontFace).arg(iFontsize).arg(sFontFace).arg(iFontsize).arg("&lt;unknown&gt;");
    }
  }
  else
  {
    sRet =
      "<p style=\"font-family:'%1';font-size:%2px\">"
        "Source: %3<br>Bundle: %4"
      "</p>";
    sRet = sRet.arg(sFontFace).arg(iFontsize);
  }

  QString sSource = data.spResource->m_sSource.toString();
  sRet = sRet.arg(sSource.isEmpty() ? "&lt;no source&gt;" : sSource)
             .arg(data.spResource->m_sResourceBundle.isEmpty() ?
                    "&lt;no bundle&gt;" : data.spResource->m_sResourceBundle);
  return sRet;
}

//----------------------------------------------------------------------------------------
//
qint32 CResourceToolTipPrivate::GetTipScreen(const QPoint& pos, QWidget* pW)
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
  if (CApplication::Instance()->desktop()->isVirtualDesktop())
  {
    return CApplication::Instance()->desktop()->screenNumber(pos);
  }
  else
  {
    return CApplication::Instance()->desktop()->screenNumber(pW);
  }
QT_WARNING_POP
}

//----------------------------------------------------------------------------------------
//
void CResourceToolTipPrivate::InstallToolTipFilter(const QPoint& pos, QWidget* pW)
{
#ifdef Q_OS_WIN32
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
  QList<QLabel*> vpLabels =
      QApplication::desktop()->screen(GetTipScreen(pos, pW))
      ->findChildren<QLabel*>("qtooltip_label");
QT_WARNING_POP
#else
  QList<QLabel*> vpLabels = pW->findChildren<QLabel*>("qtooltip_label");
#endif
  auto it = std::find_if(vpLabels.begin(), vpLabels.end(), [](QLabel* pLabel) -> bool {
      return pLabel->metaObject()->className() == QString("QTipLabel");
  });
  if (vpLabels.begin() != it)
  {
    QLabel* pTipLabel = *it;
    pTipLabel->installEventFilter(this);
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceToolTip::showResource(const QPoint& pos, const tspResource& spResource,
                                    QWidget* pW)
{
  CResourceToolTip::showResource(pos, spResource, pW, QRect());
}

//----------------------------------------------------------------------------------------
//
void CResourceToolTip::showResource(const QPoint& pos, const tspResource& spResource,
                                    QWidget* pW, const QRect& rect)
{
  showResource(pos, spResource, pW, rect, -1);
}

//----------------------------------------------------------------------------------------
//
void CResourceToolTip::showResource(const QPoint& pos, const tspResource& spResource,
                                    QWidget* pW, const QRect& rect, qint32 iMsecShowTime)
{
  CResourceToolTipPrivate::Instance()->ShowResource(pos, spResource, pW, rect, iMsecShowTime);
}

//----------------------------------------------------------------------------------------
//
bool CResourceToolTip::isVisible()
{
  return CResourceToolTipPrivate::Instance()->IsShown();
}

//----------------------------------------------------------------------------------------
//
tspResource CResourceToolTip::resource()
{
  return CResourceToolTipPrivate::Instance()->Resource();
}

//----------------------------------------------------------------------------------------
//
QPalette CResourceToolTip::palette()
{
  return QToolTip::palette();
}

//----------------------------------------------------------------------------------------
//
void CResourceToolTip::setPalette(const QPalette& palette)
{
  QToolTip::setPalette(palette);
}

//----------------------------------------------------------------------------------------
//
QFont CResourceToolTip::font()
{
  return QApplication::font("QTipLabel");
}

//----------------------------------------------------------------------------------------
//
void CResourceToolTip::setFont(const QFont& font)
{
  QApplication::setFont(font, "QTipLabel");
}

#include "ResourceToolTip.moc"
