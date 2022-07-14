#include "ResourceDetailView.h"
#include "ResourceDetailViewFetcherThread.h"
#include "ResourceTreeItem.h"
#include "ResourceTreeItemModel.h"
#include "Systems/ThreadedSystem.h"
#include <QApplication>
#include <QPainter>
#include <QPlainTextEdit>
#include <QStyledItemDelegate>
#include <QMovie>
#include <QDebug>

class CResourceDetailViewDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:

  inline CResourceDetailViewDelegate(CResourceDetailView* pView) :
    QStyledItemDelegate(pView),
    m_pView(pView),
    m_pLoading(new QMovie(this))
  {
    m_pLoading.setFileName(":/resources/gif/spinner_transparent.gif");
    connect(&m_pLoading, &QMovie::frameChanged, [this]() {
      m_pView->viewport()->update();
    });
  }

  //--------------------------------------------------------------------------------------
  //
  void paint(QPainter* pPainter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override
  {
    pPainter->save();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyle* pStyle = m_pView ? m_pView->style() : QApplication::style();

    // draw normally except text
    QStyleOptionViewItem optCopy = opt;
    optCopy.text = QString();
    pStyle->drawControl(QStyle::CE_ItemViewItem, &optCopy, pPainter, opt.widget);

    // draw icon over bg
    QAbstractItemModel* pModel = m_pView->model();
    if (nullptr != pModel)
    {
      const QModelIndex indexName = pModel->index(index.row(), resource_item::c_iColumnName, index.parent());
      const QModelIndex indexType = pModel->index(index.row(), resource_item::c_iColumnType, index.parent());

      if (index.isValid() && indexName.isValid() && indexType.isValid())
      {
        const QString sName = pModel->data(indexName).toString();
        const QVariant vType = pModel->data(indexType);
        const QVariant vItemType = pModel->data(indexType, CResourceTreeItemModel::eItemTypeRole);
        const qint32 iLoadedId = pModel->data(indexType, CResourceTreeItemModel::eLoadedIDRole).toInt();
        QIcon::Mode mode = opt.showDecorationSelected ? QIcon::Selected :
                                                        QIcon::Normal;

        QSize iconSize = m_pView->iconSize();
        QRect rectIconBounding = opt.rect;
        rectIconBounding.setHeight(iconSize.height());
        qint32 iOffsetX = (rectIconBounding.width() - iconSize.width()) / 2;
        qint32 iOffsetY = (rectIconBounding.height() - iconSize.height()) / 2;
        QRect rectIcon({opt.rect.x() + iOffsetX, opt.rect.y() + iOffsetY}, iconSize);

        // folder
        if (!vItemType.isValid() || vItemType.toInt() != EResourceTreeItemType::eResource)
        {
          m_pView->IconFolder().paint(pPainter,
                                      rectIcon, opt.displayAlignment,
                                      mode,
                                      m_pView->ReadOnly() ? QIcon::Off : QIcon::On);
        }
        // resource
        else
        {
          QFont modedFont = pPainter->font();
          modedFont.setPointSize(m_pView->iconSize().height() / 3);
          pPainter->save();
          pPainter->setFont(modedFont);
          switch(EResourceType::_from_integral(vType.toInt()))
          {
            case EResourceType::eImage:
            {
              QPixmap px = m_pView->PreviewImageForResource(sName);
              if (px.isNull())
              {
                QImage img = m_pLoading.currentImage();
                pPainter->drawImage(rectIcon, img, img.rect());
              }
              else
              {
                qint32 iXAdjust = (rectIcon.width()-px.width())/2;
                qint32 iYAdjust = (rectIcon.height()-px.height())/2;
                rectIcon.adjust(iXAdjust, iYAdjust, -iXAdjust, -iYAdjust);
                pPainter->drawPixmap(rectIcon, px, px.rect());
              }
            } break;
            case EResourceType::eMovie:
            {
              QPixmap px = m_pView->PreviewImageForResource(sName);
              if (px.isNull())
              {
                QImage img = m_pLoading.currentImage();
                pPainter->drawImage(rectIcon, img, img.rect());
              }
              else
              {
                qint32 iXAdjust = (rectIcon.width()-px.width())/2;
                qint32 iYAdjust = (rectIcon.height()-px.height())/2;
                rectIcon.adjust(iXAdjust, iYAdjust, -iXAdjust, -iYAdjust);
                pPainter->drawPixmap(rectIcon, px, px.rect());
              }
            } break;
            case EResourceType::eSound:
              m_pView->IconFile().paint(pPainter,
                                        rectIcon, opt.displayAlignment,
                                        mode,
                                        m_pView->ReadOnly() ? QIcon::Off : QIcon::On);
              pStyle->drawItemText(pPainter, rectIcon, Qt::AlignCenter,
                                   opt.palette, true, "♫");
              break;
            case EResourceType::eScript:
              m_pView->IconFile().paint(pPainter,
                                        rectIcon, opt.displayAlignment,
                                        mode,
                                        m_pView->ReadOnly() ? QIcon::Off : QIcon::On);
              pStyle->drawItemText(pPainter, rectIcon, Qt::AlignCenter,
                                   opt.palette, true, "{}");
              break;
            case EResourceType::eFont:
            {
              const QStringList vsFamilies = QFontDatabase::applicationFontFamilies(iLoadedId);
              if (vsFamilies.size() > 0)
              {
                modedFont.setFamily(vsFamilies.first());
                modedFont.setPointSize(opt.font.pointSize()*2);
                pPainter->setFont(modedFont);
              }
              m_pView->IconFile().paint(pPainter,
                                        rectIcon, opt.displayAlignment,
                                        mode,
                                        m_pView->ReadOnly() ? QIcon::Off : QIcon::On);
              pStyle->drawItemText(pPainter, rectIcon, Qt::AlignCenter,
                                   opt.palette, true, "Aa");
            } break;
            case EResourceType::eOther: // fallthrough
            case EResourceType::eDatabase:
              m_pView->IconFile().paint(pPainter,
                                        rectIcon, opt.displayAlignment,
                                        mode,
                                        m_pView->ReadOnly() ? QIcon::Off : QIcon::On);
              break;
          }
          pPainter->restore();
        }

        // and finally draw text below icon
        QRect rectText = opt.rect.adjusted(0, m_pView->iconSize().height(), 0, 0);
        pStyle->drawItemText(pPainter, rectText, opt.displayAlignment | Qt::TextWrapAnywhere,
                             opt.palette, true, opt.text);
      }
    }
    pPainter->restore();
  }

  //--------------------------------------------------------------------------------------
  //
  QSize sizeHint(
      const QStyleOptionViewItem& option, const QModelIndex& index) const override
  {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QSize size(m_pView->iconSize());
    QRect rectTextBounds = CalculateTextRect(opt);
    return QSize(rectTextBounds.size().width(), rectTextBounds.height() + size.height());
  }

  //--------------------------------------------------------------------------------------
  //
  void updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const override
  {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QRect rectTextInItem = CalculateTextRect(opt);
    pEditor->setFixedSize(rectTextInItem.size());
    QPoint rectTextOffset = m_pView->visualRect(index).topLeft();
    pEditor->move(rectTextOffset + rectTextInItem.topLeft());
  }

  //--------------------------------------------------------------------------------------
  //
  void StartLoading()
  {
    m_pLoading.start();
  }

  //--------------------------------------------------------------------------------------
  //
  void StopLoading()
  {
    m_pLoading.stop();
  }

private:
  QRect CalculateTextRect(const QStyleOptionViewItem& opt) const
  {
    QSize size(m_pView->iconSize());
    QRect rectTextBounds{0, 0, size.width()*2, size.height()};
    QSize textSize =
        opt.fontMetrics.boundingRect(rectTextBounds, Qt::TextWrapAnywhere, opt.text).size();
    qint32 iTargetWidth =
        std::min({std::max({size.width(), textSize.width()}), size.width()*2});

    size.setWidth(iTargetWidth);
    size.setHeight(textSize.height());

    return QRect{{0, m_pView->iconSize().height()}, size};
  }

  CResourceDetailView* m_pView;
  QMovie               m_pLoading;
};

//----------------------------------------------------------------------------------------
//
CResourceDetailView::CResourceDetailView(QWidget* pParent) :
  QListView(pParent),
  m_spThreadedLoader(std::make_unique<CThreadedSystem>("ResourceDetailViewFetcher")),
  m_spProject(nullptr),
  m_imageCache(),
  m_bReadOnly(false),
  m_iconFile(":/resources/style/img/FileIcon.png"),
  m_iconFolder(":/resources/style/img/FolderIcon.png")
{
  m_spThreadedLoader->RegisterObject<CResourceDetailViewFetcherThread>();

  setWrapping(true);
  setBatchSize(10);
  setLayoutMode(QListView::Batched);
  setUniformItemSizes(false);
  setViewMode(QListView::IconMode);
  setWordWrap(true);
  setResizeMode(QListView::Adjust);

  setItemDelegate(new CResourceDetailViewDelegate(this));
  connect(this, &QListView::doubleClicked, this, &CResourceDetailView::SlotDoubleClicked);

  connect(ResourceFetcher().get(), &CResourceDetailViewFetcherThread::LoadStarted,
          this, [this](){
    dynamic_cast<CResourceDetailViewDelegate*>(itemDelegate())->StartLoading();
  }, Qt::QueuedConnection);
  connect(ResourceFetcher().get(),
          qOverload<const QString&, const QPixmap&>(&CResourceDetailViewFetcherThread::LoadFinished),
          this, &CResourceDetailView::SlotResourceLoadFinished, Qt::QueuedConnection);
}

CResourceDetailView::~CResourceDetailView()
{

}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::Initialize(tspProject spProject)
{
  m_spProject = spProject;
  bool bOk = QMetaObject::invokeMethod(ResourceFetcher().get(), "Initialize", Qt::BlockingQueuedConnection);
  assert(bOk); Q_UNUSED(bOk);
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::DeInitilaze()
{
  // reset model
  ResourceFetcher()->AbortLoading();
  m_imageCache.clear();
  bool bOk = QMetaObject::invokeMethod(ResourceFetcher().get(), "Deinitialize", Qt::BlockingQueuedConnection);
  assert(bOk); Q_UNUSED(bOk);
  setRootIndex(QModelIndex());
  m_spProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::SetIconFile(const QIcon& icon)
{
  m_iconFile = icon;
}

//----------------------------------------------------------------------------------------
//
const QIcon& CResourceDetailView::IconFile() const
{
  return m_iconFile;
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::SetIconFolder(const QIcon& icon)
{
  m_iconFolder = icon;
}

//----------------------------------------------------------------------------------------
//
const QIcon& CResourceDetailView::IconFolder() const
{
  return m_iconFolder;
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::SetReadOnly(bool bReadOnly)
{
  if (m_bReadOnly != bReadOnly)
  {
    m_bReadOnly = bReadOnly;
  }
}

//----------------------------------------------------------------------------------------
//
bool CResourceDetailView::ReadOnly()
{
  return m_bReadOnly;
}

//----------------------------------------------------------------------------------------
//
const QPixmap& CResourceDetailView::PreviewImageForResource(const QString& sResource) const
{
  static QPixmap c_invalidPixmap = QPixmap();
  auto it = m_imageCache.find(sResource);
  if (m_imageCache.end() != it)
  {
    return it->second;
  }
  return c_invalidPixmap;
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::RequestResource(const QModelIndex& index)
{
  qint32 iProject = -1;
  if (nullptr != m_spProject)
  {
    QReadLocker locker(&m_spProject->m_rwLock);
    iProject = m_spProject->m_iId;
  }

  QAbstractItemModel* pModel = model();
  if (nullptr != pModel)
  {
    const QModelIndex idxName = pModel->index(index.row(), resource_item::c_iColumnName, index.parent());
    const QModelIndex idxType = pModel->index(index.row(), resource_item::c_iColumnType, index.parent());
    const QVariant varType = pModel->data(idxType);
    if (!varType.isNull())
    {
      qint32 type = varType.toInt();
      if (EResourceType::eImage == type || EResourceType::eMovie == type)
      {
        ResourceFetcher()->RequestResources(iProject, {pModel->data(idxName).toString()}, iconSize());
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::Expand(const QModelIndex& index)
{
  setRootIndex(index);
  RequestResourcesFromCurrentFolder();
  emit Expanded(index);
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::Collapse(const QModelIndex& index)
{
  if (index.isValid())
  {
    setRootIndex(index.parent());
    RequestResourcesFromCurrentFolder();
    emit Collapsed(index);
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::UpdateResources()
{
  RequestResourcesFromCurrentFolder();
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::SlotDoubleClicked(const QModelIndex& index)
{
  QAbstractItemModel* pModel = model();
  if (nullptr != pModel)
  {
    const QModelIndex indexType = pModel->index(index.row(), resource_item::c_iColumnType, index.parent());
    if (index.isValid() && indexType.isValid())
    {
      const QVariant vType = pModel->data(indexType);
      if (!vType.isValid())
      {
        Expand(index);
      }
      else
      {
        QListView::edit(index);
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::SlotResourceLoadFinished(const QString& sResource, const QPixmap& pixmap)
{
  if (!ResourceFetcher()->IsLoading())
  {
    dynamic_cast<CResourceDetailViewDelegate*>(itemDelegate())->StopLoading();
  }

  if (!sResource.isEmpty() && !pixmap.isNull())
  {
    m_imageCache.insert({sResource, pixmap});
  }
  viewport()->update();
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::dataChanged(const QModelIndex& topLeft,
                                      const QModelIndex& bottomRight,
                                      const QVector<int>& viRoles)
{
  QListView::dataChanged(topLeft, bottomRight, viRoles);

  qint32 iProject = -1;
  if (nullptr != m_spProject)
  {
    QReadLocker locker(&m_spProject->m_rwLock);
    iProject = m_spProject->m_iId;
  }

  QAbstractItemModel* pModel = model();
  QStringList vsResourcesToRequest;
  if (nullptr != pModel)
  {
    const QModelIndex idxName = pModel->index(topLeft.row(), resource_item::c_iColumnName, topLeft.parent());
    const QModelIndex idxType = pModel->index(topLeft.row(), resource_item::c_iColumnType, topLeft.parent());
    const QVariant varType = pModel->data(idxType);
    if (!varType.isNull())
    {
      qint32 type = varType.toInt();
      if (EResourceType::eImage == type || EResourceType::eMovie == type)
      {
        vsResourcesToRequest << pModel->data(idxName).toString();
      }
    }
  }

  ResourceFetcher()->RequestResources(iProject, vsResourcesToRequest, iconSize());
}

//----------------------------------------------------------------------------------------
//
bool CResourceDetailView::edit(const QModelIndex& index, EditTrigger trigger, QEvent* pEvent)
{
  if (!ReadOnly())
  {
    return QListView::edit(index, trigger, pEvent);
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CResourceDetailView::RequestResourcesFromCurrentFolder()
{
  ResourceFetcher()->AbortLoading();
  m_imageCache.clear();

  qint32 iProject = -1;
  if (nullptr != m_spProject)
  {
    QReadLocker locker(&m_spProject->m_rwLock);
    iProject = m_spProject->m_iId;
  }

  QAbstractItemModel* pModel = model();
  QModelIndex idx = rootIndex();
  QStringList vsResourcesToRequest;
  if (nullptr != pModel)
  {
    if (pModel->hasChildren(idx))
    {
      for (qint32 i = 0; pModel->rowCount(idx) > i; ++i)
      {
        const QModelIndex idxName = pModel->index(i, resource_item::c_iColumnName, idx);
        const QModelIndex idxType = pModel->index(i, resource_item::c_iColumnType, idx);
        const QVariant varType = pModel->data(idxType);
        if (!varType.isNull())
        {
          qint32 type = varType.toInt();
          if (EResourceType::eImage == type || EResourceType::eMovie == type)
          {
            vsResourcesToRequest << pModel->data(idxName).toString();
          }
        }
      }
    }

    ResourceFetcher()->RequestResources(iProject, vsResourcesToRequest, iconSize());
  }
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CResourceDetailViewFetcherThread> CResourceDetailView::ResourceFetcher() const
{
  return std::static_pointer_cast<CResourceDetailViewFetcherThread>(m_spThreadedLoader->Get());
}

// private class CResourceDetailViewDelegate
#include "ResourceDetailView.moc"
