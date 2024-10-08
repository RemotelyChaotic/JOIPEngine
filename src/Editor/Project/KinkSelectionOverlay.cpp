#include "KinkSelectionOverlay.h"
#include "KinkTreeModel.h"
#include "KinkTreeSortFilterProxyModel.h"
#include "ui_KinkSelectionOverlay.h"
#include <QSortFilterProxyModel>

CKinkSelectionOverlay::CKinkSelectionOverlay(QWidget* pParent) :
  COverlayBase(2, pParent),
  m_spUi(std::make_unique<Ui::CKinkSelectionOverlay>()),
  m_bInitialized(false)
{
  m_spUi->setupUi(this);
  CKinkTreeSortFilterProxyModel* pProxyModel = new CKinkTreeSortFilterProxyModel(m_spUi->pKinkTree);
  m_spUi->pKinkTree->setModel(pProxyModel);
}

CKinkSelectionOverlay::~CKinkSelectionOverlay()
{
  dynamic_cast<CKinkTreeSortFilterProxyModel*>(m_spUi->pKinkTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CKinkSelectionOverlay::Initialize(CKinkTreeModel* pKinkTreeModel)
{
  m_bInitialized = false;

  CKinkTreeSortFilterProxyModel* pProxyModel =
      dynamic_cast<CKinkTreeSortFilterProxyModel*>(m_spUi->pKinkTree->model());
  pProxyModel->setSourceModel(pKinkTreeModel);

  pProxyModel->sort(0, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  m_spUi->pKinkTree->header()->stretchLastSection();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CKinkSelectionOverlay::LoadProject(bool bReadOnly)
{
  CKinkTreeSortFilterProxyModel* pProxyModel =
      dynamic_cast<CKinkTreeSortFilterProxyModel*>(m_spUi->pKinkTree->model());
  CKinkTreeModel* pSourceModel = dynamic_cast<CKinkTreeModel*>(pProxyModel->sourceModel());
  pSourceModel->SetReadOnly(bReadOnly);
}

//----------------------------------------------------------------------------------------
//
void CKinkSelectionOverlay::UnloadProject()
{
  CKinkTreeSortFilterProxyModel* pProxyModel =
      dynamic_cast<CKinkTreeSortFilterProxyModel*>(m_spUi->pKinkTree->model());
  CKinkTreeModel* pSourceModel = dynamic_cast<CKinkTreeModel*>(pProxyModel->sourceModel());
  pSourceModel->SetReadOnly(false);
}

//----------------------------------------------------------------------------------------
//
void CKinkSelectionOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}

//----------------------------------------------------------------------------------------
//
void CKinkSelectionOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CKinkSelectionOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
{
  if (!m_bInitialized) { return; }

  QSortFilterProxyModel* pProxyModel =
      dynamic_cast<QSortFilterProxyModel*>(m_spUi->pKinkTree->model());

  if (sText.isNull() || sText.isEmpty())
  {
    pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));
  }
  else
  {
    pProxyModel->setFilterRegExp(QRegExp(sText, Qt::CaseInsensitive, QRegExp::RegExp));
  }
}

//----------------------------------------------------------------------------------------
//
void CKinkSelectionOverlay::on_pConfirmButton_clicked()
{
  Hide();
}
