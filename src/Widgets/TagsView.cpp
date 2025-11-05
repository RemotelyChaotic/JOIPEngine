#include "TagsView.h"
#include "FlowLayout.h"
#include "ui_TagsView.h"

#include "Systems/Database/Tag.h"

#include <QDebug>
#include <QLabel>
#include <QPushButton>

CTagsView::CTagsView(QWidget *parent) :
    QFrame(parent),
    m_spUi(std::make_unique<Ui::CTagsView>()),
    m_vspTags(),
    m_fnAdded(nullptr),
    m_fnRemoved(nullptr),
    m_fnSort(nullptr)
{
  m_spUi->setupUi(this);
  CFlowLayout* pFlow = new CFlowLayout(this, 6, 4, 4);
  setLayout(pFlow);
}

CTagsView::~CTagsView()
{
}

//----------------------------------------------------------------------------------------
//
void CTagsView::SetCallbacks(tfnAdded fnAdded, tfnRemoved fnRemoved)
{
  m_fnAdded = fnAdded;
  m_fnRemoved = fnRemoved;
}

//----------------------------------------------------------------------------------------
//
void CTagsView::SetSortFunction(tfnSort fnSort)
{
  m_fnSort = fnSort;
}

//----------------------------------------------------------------------------------------
//
void CTagsView::SetFontSize(qint32 iFontSize)
{
  m_iFontSize = iFontSize;
}

//----------------------------------------------------------------------------------------
//
void CTagsView::SetReadOnly(bool bReadOnly)
{
  m_bReadOnly = bReadOnly;
}

//----------------------------------------------------------------------------------------
//
void CTagsView::AddTags(const std::vector<std::shared_ptr<SLockableTagData>>& vspTags)
{
  CFlowLayout* pLayout = dynamic_cast<CFlowLayout*>(layout());
  if (nullptr != pLayout)
  {
    for (const auto& spTag : vspTags)
    {
      m_vspTags.push_back(spTag);
    }

    if (nullptr != m_fnSort)
    {
      m_fnSort(m_vspTags);
    }

    for (const auto& spTag : vspTags)
    {
      if (nullptr == spTag) { continue; }

      qint32 iIndexOfNewElem =
          std::find(m_vspTags.begin(), m_vspTags.end(), spTag) - m_vspTags.begin();

      // Farbe für Kategorie ausrechnen
      QColor hashColor = CalculateTagColor(*spTag);

      // calculate foreground / text color
      double dLuminance = (0.299 * hashColor.red() +
                           0.587 * hashColor.green() +
                           0.114 * hashColor.blue()) / 255;
      QColor foregroundColor = Qt::white;
      if (dLuminance > 0.5)
      {
        foregroundColor = Qt::black;
      }

      QReadLocker locker(&spTag->m_rwLock);
      QWidget* pRoot = new QWidget(this);
      QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      sizePolicy.setHorizontalStretch(0);
      sizePolicy.setVerticalStretch(0);
      sizePolicy.setHeightForWidth(pRoot->sizePolicy().hasHeightForWidth());
      pRoot->setSizePolicy(sizePolicy);
      pRoot->setObjectName(spTag->m_sName);
      pRoot->setStyleSheet(QString("QWidget { background-color: %1; border-radius: 5px; }")
                               .arg(hashColor.name()));
      pRoot->setToolTip(!spTag->m_sDescribtion.isEmpty() ?
                        spTag->m_sDescribtion : tr("No Describtion available."));

      QHBoxLayout* pRootLayout = new QHBoxLayout(pRoot);
      pRootLayout->setContentsMargins({2,2,2,2});
      pRootLayout->setMargin(2);
      pRoot->setLayout(pRootLayout);

      QLabel* pLabel = new QLabel(spTag->m_sName, pRoot);
      QFont fontLabel = pLabel->font();
      fontLabel.setPointSize(m_iFontSize);
      pLabel->setFont(fontLabel);
      pLabel->setStyleSheet(QString("QLabel { background-color: transparent; color: %1; }")
                                .arg(foregroundColor.name()));
      pLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
      pRootLayout->addWidget(pLabel);

      QPushButton* pRemove = nullptr;
      if (!m_bReadOnly)
      {
        pRemove = new QPushButton(pRoot);
        pRemove->setObjectName("RemoveFetishButtonX");
        pRemove->setText("X");
        pRemove->setFlat(true);
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(pRemove->sizePolicy().hasHeightForWidth());
        pRemove->setSizePolicy(sizePolicy3);
        //pRemove->setMinimumSize(QSize(24, 24));
        //pRemove->setMaximumSize(QSize(24, 24));
        pRemove->setStyleSheet(QString("QPushButton { background-color: transparent; color: %1;"
                                       "border: none; padding: 2px; min-width: 1px; min-height: 1px; border-image: none; }")
                                   .arg(foregroundColor.name()));
        pRootLayout->addWidget(pRemove);
      }

      pLayout->insertWidget(iIndexOfNewElem, pRoot);

      if (nullptr != m_fnAdded)
      {
        m_fnAdded(pRemove, spTag->m_sName);
      }
    }
  }
  else
  {
    qWarning() << tr("pFetishListWidget has no FlowLayout.");
  }
}

//----------------------------------------------------------------------------------------
//
void CTagsView::ClearTags()
{
  CFlowLayout* pLayout = dynamic_cast<CFlowLayout*>(layout());
  if (nullptr != pLayout)
  {
    while (QLayoutItem* pItem = pLayout->takeAt(0))
    {
      if (nullptr != pItem->widget())
      {
        delete pItem->widget();
        delete pItem;
      }
    }

    pLayout->update();
    m_vspTags.clear();
  }
}

//----------------------------------------------------------------------------------------
//
void CTagsView::RemoveTags(QStringList vsTags)
{
  QStringList vsRemoved;
  for (const QString& sTag : qAsConst(vsTags))
  {
    CFlowLayout* pLayout = dynamic_cast<CFlowLayout*>(layout());
    if (nullptr != pLayout)
    {
      for (qint32 i = 0; pLayout->count() > i; ++i)
      {
        if (nullptr != pLayout->itemAt(i)->widget() &&
            pLayout->itemAt(i)->widget()->objectName() == sTag)
        {
          QLayoutItem* pItem = pLayout->takeAt(i);
          delete pItem->widget();
          delete pItem;
          break;
        }
      }

      for (auto it = m_vspTags.begin(); m_vspTags.end() != it; ++it)
      {
        QReadLocker locker(&(*it)->m_rwLock);
        if ((*it)->m_sName == sTag)
        {
          m_vspTags.erase(it);
          break;
        }
      }


      pLayout->update();
      vsRemoved << sTag;
    }
  }

  if (nullptr != m_fnRemoved && vsRemoved.size() > 0)
  {
    m_fnRemoved(vsRemoved);
  }
}

//----------------------------------------------------------------------------------------
//
void CTagsView::UpdateToolTip(const QString& sTag, const QString& sDescribtion)
{
  CFlowLayout* pLayout = dynamic_cast<CFlowLayout*>(layout());
  if (nullptr != pLayout)
  {
    for (qint32 i = 0; pLayout->count() > i; ++i)
    {
      QLayoutItem* pItem = pLayout->itemAt(i);
      if (nullptr != pItem && nullptr != pItem->widget())
      {
        if (pItem->widget()->objectName() == sTag)
        {
          pItem->widget()->setToolTip(!sDescribtion.isEmpty() ?
                                      sDescribtion : tr("No Describtion available."));
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
const std::vector<std::shared_ptr<SLockableTagData>>& CTagsView::Tags() const
{
  return m_vspTags;
}
