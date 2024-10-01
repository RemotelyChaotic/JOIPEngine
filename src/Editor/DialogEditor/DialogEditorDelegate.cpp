#include "DialogEditorDelegate.h"
#include "CommandChangeModelViaGui.h"
#include "DialogEditorTreeItem.h"
#include "DialogEditorTreeModel.h"

#include "Editor/EditorModel.h"

#include "Widgets/LongLongSpinBox.h"

#include <QComboBox>
#include <QFileInfo>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSortFilterProxyModel>

namespace
{
  const qint32 c_iIdxNewFile = Qt::UserRole+1;
}

//----------------------------------------------------------------------------------------
//
CDialogEditorDelegate::CDialogEditorDelegate(QTreeView* pTree) :
  CHtmlViewDelegate(pTree),
  m_pParent(pTree)
{
}
CDialogEditorDelegate::~CDialogEditorDelegate() = default;

//----------------------------------------------------------------------------------------
//
void CDialogEditorDelegate::SetCurrentProject(const tspProject& spProject)
{
  m_spProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CDialogEditorDelegate::SetUndoStack(QPointer<QUndoStack> pUndo)
{
  m_pUndo = pUndo;
}

//----------------------------------------------------------------------------------------
//
QWidget* CDialogEditorDelegate::createEditor(QWidget* pParent,
                                             const QStyleOptionViewItem& option,
                                             const QModelIndex& index) const
{
  if (!index.isValid()) { return nullptr; }

  QWidget* pWidget = nullptr;
  switch (index.column())
  {
    case dialog_item::c_iColumnId: [[fallthrough]];
    case dialog_item::c_iColumnString:
    {
      QLineEdit* pLineEdit = new QLineEdit(pParent);
      pWidget = pLineEdit;
      connect(pLineEdit, &QLineEdit::editingFinished, pLineEdit, [this, pWidget]() mutable {
        emit const_cast<CDialogEditorDelegate*>(this)->commitData(pWidget);
      });
    } break;
    case dialog_item::c_iColumnWaitMS:
    {
      CLongLongSpinBox* pSpinbox = new CLongLongSpinBox(pParent);
      pWidget = pSpinbox;
      pSpinbox->setMinimum(-1);
      pSpinbox->setMaximum(std::numeric_limits<qint64>::max());
      connect(pSpinbox, &CLongLongSpinBox::valueChanged, pSpinbox, [this, pWidget]() mutable {
        emit const_cast<CDialogEditorDelegate*>(this)->commitData(pWidget);
      });
    } break;
    case dialog_item::c_iColumnMedia:
    {
      QComboBox* pCombo = new QComboBox(pParent);
      pWidget = pCombo;
      connect(pCombo, qOverload<int>(&QComboBox::currentIndexChanged), pCombo, [this, pWidget]() mutable {
        emit const_cast<CDialogEditorDelegate*>(this)->commitData(pWidget);
      });
    } break;
    default: break;
  }
  return pWidget;
}

//----------------------------------------------------------------------------------------
//
void CDialogEditorDelegate::setEditorData(QWidget* pEditor, const QModelIndex& index) const
{
  if (!index.isValid()) { return; }

  switch (index.column())
  {
    case dialog_item::c_iColumnId: [[fallthrough]];
    case dialog_item::c_iColumnString:
    {
      QLineEdit* pWidget = dynamic_cast<QLineEdit*>(pEditor);
      if (nullptr != pWidget)
      {
        QSignalBlocker b(pWidget);
        pWidget->setText(index.data(Qt::EditRole).toString());
      }
    } break;
    case dialog_item::c_iColumnWaitMS:
    {
      CLongLongSpinBox* pWidget = dynamic_cast<CLongLongSpinBox*>(pEditor);
      if (nullptr != pWidget)
      {
        QSignalBlocker b(pWidget);
        pWidget->setValue(index.data(Qt::EditRole).toLongLong());
      }
    } break;
    case dialog_item::c_iColumnMedia:
    {
      QComboBox* pWidget = dynamic_cast<QComboBox*>(pEditor);
      if (nullptr != pWidget)
      {
        QSignalBlocker b(pWidget);
        pWidget->clear();

        QReadLocker locker(&m_spProject->m_rwLock);
        pWidget->addItem("<Empty>", QString());
        pWidget->setItemData(pWidget->count()-1, false, c_iIdxNewFile);
        //pWidget->addItem("<New Resource>", QString());
        //pWidget->setItemData(pWidget->count()-1, true, c_iIdxNewFile);
        for (const auto& [sName, spResource] : m_spProject->m_spResourcesMap)
        {
          QReadLocker l(&spResource->m_rwLock);
          if (EResourceType::eSound != spResource->m_type._to_integral()) { continue; }
          pWidget->addItem(sName, sName);
          pWidget->setItemData(pWidget->count()-1, false, c_iIdxNewFile);
        }

        const QString sSelected = index.data(Qt::EditRole).toString();
        qint32 iIdx = pWidget->findData(sSelected);
        if (-1 != iIdx)
        {
          pWidget->setCurrentIndex(iIdx);
        }
      }
    } break;
    default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogEditorDelegate::setModelData(QWidget* pEditor,
                                         QAbstractItemModel* pModel,
                                         const QModelIndex& index) const
{
  if (!index.isValid() || nullptr == pEditor) { return; }

  QVariant newValue;
  const QString sHeader =
      pModel->headerData(index.column(), Qt::Horizontal, Qt::DisplayRole).toString();
  QVariant oldValue = index.data(Qt::EditRole);

  QStringList vsPath;
  CDialogEditorTreeModel* pTree = nullptr;
  QSortFilterProxyModel* pSourtFilter = dynamic_cast<QSortFilterProxyModel*>(pModel);
  if (nullptr != pSourtFilter)
  {
    pTree = dynamic_cast<CDialogEditorTreeModel*>(pSourtFilter->sourceModel());
    if (nullptr != pTree)
    {
      vsPath = pTree->Path(pSourtFilter->mapToSource(index));
    }
  }
  else if ((pTree = dynamic_cast<CDialogEditorTreeModel*>(pModel)))
  {
    vsPath = pTree->Path(index);
  }

  switch (index.column())
  {
    case dialog_item::c_iColumnId: [[fallthrough]];
    case dialog_item::c_iColumnString:
    {
      QLineEdit* pWidget = dynamic_cast<QLineEdit*>(pEditor);
      if (nullptr != pWidget)
      {
        newValue = pWidget->text();
      }
    } break;
    case dialog_item::c_iColumnWaitMS:
    {
      CLongLongSpinBox* pWidget = dynamic_cast<CLongLongSpinBox*>(pEditor);
      if (nullptr != pWidget)
      {
        newValue = pWidget->value();
      }
    } break;
    case dialog_item::c_iColumnMedia:
    {
      QComboBox* pWidget = dynamic_cast<QComboBox*>(pEditor);
      if (nullptr != pWidget)
      {
        newValue = pWidget->itemData(pWidget->currentIndex());
      }
    } break;
    default: return;
  }

  if (nullptr != m_pUndo)
  {
    m_pUndo->push(new CCommandChangeModelViaGui(oldValue, newValue, sHeader, vsPath, pTree));
  }
  else
  {
    pModel->setData(index, newValue, Qt::EditRole);
  }
}
