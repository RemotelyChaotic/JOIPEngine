#include "SequenceEmentList.h"
#include "SqeuenceElementListModel.h"

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

  const char c_sCategoryNameMetronome[] = QT_TR_NOOP("Metronome Commands");
  const char c_sCategoryNameToy[] = QT_TR_NOOP("Toy Commands");
  const char c_sCategoryNameResource[] = QT_TR_NOOP("Resource Commands");
  const char c_sCategoryNameText[] = QT_TR_NOOP("Text Commands");
  const char c_sCategoryNameScript[] = QT_TR_NOOP("Script Commands");

  const std::map<ESequenceElemCategory, QString> c_vsAllCategoryStrings = {
      { eMetronome, c_sCategoryNameMetronome },
      { eToy, c_sCategoryNameToy },
      { eResource, c_sCategoryNameResource },
      { eText, c_sCategoryNameText },
      { eScript, c_sCategoryNameScript }
  };

  //--------------------------------------------------------------------------------------
  //
  void AddChildrenToMetronomeCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr("Single Beat"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Start Pattern"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Stop Pattern"));
    pChildren << pItemChild;

    pItem->insertRows(0, pChildren);
  }
  void AddChildrenToToyCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr("Vibrate"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Linear Toy Command"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Rotate Toy Command"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Stop Vibrations"));
    pChildren << pItemChild;

    pItem->insertRows(0, pChildren);
  }
  void AddChildrenToResourceCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr("Show"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Play Video"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Pause Video"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Play Audio"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Pause Audio"));
    pChildren << pItemChild;

    pItem->insertRows(0, pChildren);
  }
  void AddChildrenToTextCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr("Show Text"));
    pChildren << pItemChild;

    pItem->insertRows(0, pChildren);
  }
  void AddChildrenToScriptCategory(QStandardItem* pItem)
  {
    QList<QStandardItem*> pChildren;

    QStandardItem* pItemChild = new QStandardItem(QObject::tr("Run Script"));
    pChildren << pItemChild;
    pItemChild = new QStandardItem(QObject::tr("Eval"));
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
  CSqeuenceElementSortFilterProxyModel* pProxy = new CSqeuenceElementSortFilterProxyModel(this);
  pProxy->setSourceModel(m_pModel);
  setModel(pProxy);
  setItemDelegate(new CSequenceEmentDelegate(this));

  setHeaderHidden(true);
}

CSequenceEmentList::~CSequenceEmentList()
{}

//----------------------------------------------------------------------------------------
//
void CSequenceEmentList::Initialize()
{
  for (const auto& [category, sCategory] : c_vsAllCategoryStrings)
  {
    QStandardItem* pItem = new QStandardItem(sCategory);
    AddChildrenToCategory(pItem, category);
    m_pModel->appendRow(pItem);
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
