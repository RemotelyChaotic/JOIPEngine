#include "SequenceEmentList.h"
#include "SqeuenceElementListModel.h"

#include "Systems/Sequence/Sequence.h"

#include <QStyledItemDelegate>

namespace
{
  enum ESequenceElemCategory
  {
    eMetronome,
    eToy,
    eResource,
    eText,
    eScript
  };

  const std::map<ESequenceElemCategory, QString> c_vsAllCategoryStrings = {
      { eMetronome, sequence::c_sCategoryIdBeat },
      { eToy, sequence::c_sCategoryIdToy },
      { eResource, sequence::c_sCategoryIdResource },
      { eText, sequence::c_sCategoryIdText },
      { eScript, sequence::c_sCategoryIdScript }
  };

  const qint32 c_iRoleId = Qt::UserRole+1;

  //--------------------------------------------------------------------------------------
  //
  void AddChildrenToMetronomeCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdBeat));
    pItemChild->setData(sequence::c_sInstructionIdBeat, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdStartPattern));
    pItemChild->setData(sequence::c_sInstructionIdStartPattern, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdStopPattern));
    pItemChild->setData(sequence::c_sInstructionIdStopPattern, c_iRoleId);
    pChildren << pItemChild;

    pItem->insertRows(0, pChildren);
  }
  void AddChildrenToToyCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdVibrate));
    pItemChild->setData(sequence::c_sInstructionIdVibrate, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdLinearToy));
    pItemChild->setData(sequence::c_sInstructionIdLinearToy, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdRotateToy));
    pItemChild->setData(sequence::c_sInstructionIdRotateToy, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdStopVibrations));
    pItemChild->setData(sequence::c_sInstructionIdStopVibrations, c_iRoleId);
    pChildren << pItemChild;

    pItem->insertRows(0, pChildren);
  }
  void AddChildrenToResourceCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdShow));
    pItemChild->setData(sequence::c_sInstructionIdShow, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdPlayVideo));
    pItemChild->setData(sequence::c_sInstructionIdPlayVideo, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdPauseVideo));
    pItemChild->setData(sequence::c_sInstructionIdPauseVideo, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdPlayAudio));
    pItemChild->setData(sequence::c_sInstructionIdPlayAudio, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdPauseAudio));
    pItemChild->setData(sequence::c_sInstructionIdPauseAudio, c_iRoleId);
    pChildren << pItemChild;

    pItem->insertRows(0, pChildren);
  }
  void AddChildrenToTextCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdShowText));
    pItemChild->setData(sequence::c_sInstructionIdShowText, c_iRoleId);
    pChildren << pItemChild;

    pItem->insertRows(0, pChildren);
  }
  void AddChildrenToScriptCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdRunScript));
    pItemChild->setData(sequence::c_sInstructionIdRunScript, c_iRoleId);
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr(sequence::c_sInstructionIdEval));
    pItemChild->setData(sequence::c_sInstructionIdEval, c_iRoleId);
    pChildren << pItemChild;

    pItem->insertRows(0, pChildren);
  }

  //--------------------------------------------------------------------------------------
  //
  void AddChildrenToCategory(QStandardItem* pItem, ESequenceElemCategory category)
  {
    switch (category)
    {
      case ESequenceElemCategory::eMetronome:
        AddChildrenToMetronomeCategory(pItem);
        break;
      case ESequenceElemCategory::eToy:
        AddChildrenToToyCategory(pItem);
        break;
      case ESequenceElemCategory::eResource:
        AddChildrenToResourceCategory(pItem);
        break;
      case ESequenceElemCategory::eText:
        AddChildrenToTextCategory(pItem);
        break;
      case ESequenceElemCategory::eScript:
        AddChildrenToScriptCategory(pItem);
        break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
class CSequenceEmentDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:

  explicit CSequenceEmentDelegate(CSequenceEmentList* pView) :
      QStyledItemDelegate(pView),
      m_pView(pView)
  {}
  void paint(QPainter* pPainter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(
      const QStyleOptionViewItem& option, const QModelIndex& index) const override;

protected:
  void initStyleOption(QStyleOptionViewItem* pOption,
                       const QModelIndex& index) const override;

private:
  CSequenceEmentList* m_pView;
};

//----------------------------------------------------------------------------------------
//
void CSequenceEmentDelegate::paint(QPainter* pPainter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
{
  QStyledItemDelegate::paint(pPainter, option, index);
}

//----------------------------------------------------------------------------------------
//
QSize CSequenceEmentDelegate::sizeHint(
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  return QStyledItemDelegate::sizeHint(option, index);
}

//----------------------------------------------------------------------------------------
//
void CSequenceEmentDelegate::initStyleOption(QStyleOptionViewItem* pOption,
                                             const QModelIndex& index) const
{
  QStyledItemDelegate::initStyleOption(pOption, index);
}

//----------------------------------------------------------------------------------------
//
CSequenceEmentList::CSequenceEmentList(QWidget *parent) :
  QTreeView{parent},
  m_pModel(new CSqeuenceElementListModel(this))
{
  for (const auto& [_, sCategory] : c_vsAllCategoryStrings)
  {
    m_vsCategories << sCategory;
  }

  m_pSortFilter = new CSqeuenceElementSortFilterProxyModel(this);
  m_pSortFilter->setSourceModel(m_pModel);
  setModel(m_pSortFilter);
  setItemDelegate(new CSequenceEmentDelegate(this));

  setHeaderHidden(true);
  setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(selectionModel(), &QItemSelectionModel::currentChanged,
          this, &CSequenceEmentList::SlotSelectionChanged);
}

CSequenceEmentList::~CSequenceEmentList()
{}

//----------------------------------------------------------------------------------------
//
void CSequenceEmentList::Initialize()
{
  for (const auto& [category, sCategory] : c_vsAllCategoryStrings)
  {
    if (m_vsCategories.contains(sCategory))
    {
      QStandardItem* pItem = new QStandardItem(sCategory);
      AddChildrenToCategory(pItem, category);
      m_pModel->appendRow(pItem);
    }
  }

  ExpandAll();
}

//----------------------------------------------------------------------------------------
//
void CSequenceEmentList::Deinitalize()
{
  m_pModel->clear();
}

//----------------------------------------------------------------------------------------
//
void CSequenceEmentList::ExpandAll()
{
  IterateItems(QModelIndex(), [this](const QModelIndex& idx) {
    expand(idx);
  });
}

//----------------------------------------------------------------------------------------
//
void CSequenceEmentList::SetAllowedCategories(const QStringList& vsCategories)
{
  m_vsCategories = vsCategories;
}

//----------------------------------------------------------------------------------------
//
void CSequenceEmentList::SetFilter(const QString& sFilter)
{
  if (sFilter.isEmpty())
  {
    m_pSortFilter->setFilterRegExp(QRegExp(".*"));
  }
  else
  {
    m_pSortFilter->setFilterRegExp(QRegExp(sFilter, Qt::CaseInsensitive));
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceEmentList::SlotSelectionChanged(const QModelIndex& current,
                                              const QModelIndex&)
{
  if (current.isValid() && !m_pSortFilter->hasChildren(current))
  {
    emit SignalSelectedItem(current.data(c_iRoleId).toString());
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceEmentList::IterateItems(const QModelIndex& index,
                                      const std::function<void(const QModelIndex&)>& fnToCall)
{
  if (index.isValid())
  {
    fnToCall(index);
  }

  if (!model()->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren))
  {
    return;
  }
  qint32 iRows = model()->rowCount(index);
  for (qint32 i = 0; i < iRows; ++i)
  {
    IterateItems(model()->index(i, 0, index), fnToCall);
  }
}

#include "SequenceEmentList.moc"
