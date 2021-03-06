#include "ResourceSnippetOverlay.h"
#include "Application.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"
#include "ui_ResourceSnippetOverlay.h"

CResourceSnippetOverlay::CResourceSnippetOverlay(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_spUi(std::make_unique<Ui::CResourceSnippetOverlay>()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_bInitialized(false),
  m_data()
{
  m_spUi->setupUi(this);
  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      new CResourceTreeItemSortFilterProxyModel(m_spUi->pResourceSelectTree);
  m_spUi->pResourceSelectTree->setModel(pProxyModel);
}

CResourceSnippetOverlay::~CResourceSnippetOverlay()
{
  dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model())
      ->setSourceModel(nullptr);
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::Initialize(CResourceTreeItemModel* pResourceTreeModel)
{
  m_bInitialized = false;

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  pProxyModel->setSourceModel(pResourceTreeModel);

  QItemSelectionModel* pSelectionModel = m_spUi->pResourceSelectTree->selectionModel();
  connect(pSelectionModel, &QItemSelectionModel::currentChanged,
          this, &CResourceSnippetOverlay::SlotCurrentChanged);

  pProxyModel->sort(resource_item::c_iColumnName, Qt::AscendingOrder);
  pProxyModel->setFilterRegExp(QRegExp(".*", Qt::CaseInsensitive, QRegExp::RegExp));

  // setup Tree
  m_spUi->pResourceSelectTree->setColumnHidden(resource_item::c_iColumnPath, true);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnName, QHeaderView::Stretch);
  m_spUi->pResourceSelectTree->header()->setSectionResizeMode(resource_item::c_iColumnType, QHeaderView::Interactive);
  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::LoadProject(tspProject spProject)
{
  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::UnloadProject()
{
  m_spCurrentProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pResourceLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  m_data.m_sResource = m_spUi->pResourceLineEdit->text();
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_CloseButton_clicked()
{
  if (!m_bInitialized) { return; }
  m_spUi->pResourceLineEdit->clear();
  m_data.m_sResource = QString();
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pPlayRadioButton_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  if (bChecked)
  {
    m_data.m_displayMode = EDisplayMode::ePlayShow;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pPauseRadioButton_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  if (bChecked)
  {
    m_data.m_displayMode = EDisplayMode::ePause;
  }
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pStopRadioButton_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  if (bChecked)
  {
    m_data.m_displayMode = EDisplayMode::eStop;
  }
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pWaitForFinishedCheckBox_toggled(bool bChecked)
{
  if (!m_bInitialized) { return; }
  m_data.m_bWaitForFinished = bChecked;
}


//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pFilter_SignalFilterChanged(const QString& sText)
{
  if (!m_bInitialized) { return; }

  CResourceTreeItemSortFilterProxyModel* pProxyModel =
      dynamic_cast<CResourceTreeItemSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());

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
void CResourceSnippetOverlay::on_pConfirmButton_clicked()
{
  EResourceType type = EResourceType::eImage;
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager && nullptr != m_spCurrentProject)
  {
    tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, m_data.m_sResource);
    if (nullptr != spResource)
    {
      QReadLocker locker(&spResource->m_rwLock);
      type = spResource->m_type;
    }
  }

  QString sCode;
  if (!m_data.m_sResource.isEmpty())
  {
    if (EDisplayMode::ePlayShow == m_data.m_displayMode)
    {
      QString sMainCommand("mediaPlayer.%1(\"%2\");\n");
      switch (type)
      {
        case EResourceType::eImage:
          sMainCommand = sMainCommand.arg("show");
          break;
        case EResourceType::eMovie:
          sMainCommand = sMainCommand.arg("play");
          break;
        case EResourceType::eSound:
          sMainCommand = sMainCommand.arg("playSound");
          break;
        default: break;
      }
      sMainCommand = sMainCommand.arg(m_data.m_sResource);
      sCode += sMainCommand;
    }
    else if (EDisplayMode::ePause == m_data.m_displayMode)
    {
      switch (type)
      {
        case EResourceType::eImage: break;
        case EResourceType::eMovie: sCode += "mediaPlayer.pauseVideo();\n"; break;
        case EResourceType::eSound:
          sCode += QString("mediaPlayer.pauseSound(\"%1\");\n").arg(m_data.m_sResource);
          break;
        default: break;
      }
    }
    else if (EDisplayMode::eStop == m_data.m_displayMode)
    {
      switch (type)
      {
        case EResourceType::eImage: break;
        case EResourceType::eMovie: sCode += "mediaPlayer.stopVideo();\n"; break;
        case EResourceType::eSound:
          sCode += QString("mediaPlayer.stopSound(\"%1\");\n").arg(m_data.m_sResource);
          break;
        default: break;
      }
    }
  }
  else
  {
    if (EDisplayMode::ePlayShow == m_data.m_displayMode)
    {
      sCode += "mediaPlayer.play();\n";
    }
    else if (EDisplayMode::ePause == m_data.m_displayMode)
    {
      sCode += "mediaPlayer.pauseVideo();\n";
      sCode += "mediaPlayer.pauseSound();\n";
    }
    else if (EDisplayMode::eStop == m_data.m_displayMode)
    {
      sCode +="mediaPlayer.stopVideo();\n";
      sCode +="mediaPlayer.stopSound();\n";
    }
  }

  if ((type._to_integral() == EResourceType::eMovie || type._to_integral() == EResourceType::eSound)
      && m_data.m_bWaitForFinished)
  {
    if (!m_data.m_sResource.isEmpty())
    {
      switch (type)
      {
        case EResourceType::eImage:
          sCode += QString("mediaPlayer.waitForPlayback(\"%1\");\n").arg(m_data.m_sResource);
          break;
        case EResourceType::eMovie:
          sCode += QString("mediaPlayer.waitForVideo();\n").arg(m_data.m_sResource);
          break;
        case EResourceType::eSound:
          sCode += QString("mediaPlayer.waitForSound(\"%1\");\n").arg(m_data.m_sResource);
          break;
        default: break;
      }
    }
    else
    {
      sCode += "mediaPlayer.waitForPlayback();\n";
    }
  }

  emit SignalResourceCode(sCode);
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::on_pCancelButton_clicked()
{
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CResourceSnippetOverlay::SlotCurrentChanged(const QModelIndex& current,
                                                   const QModelIndex& previous)
{
  if (!m_bInitialized) { return; }
  Q_UNUSED(previous);

  QSortFilterProxyModel* pProxyModel =
    dynamic_cast<QSortFilterProxyModel*>(m_spUi->pResourceSelectTree->model());
  CResourceTreeItemModel* pModel =
    dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

  if (nullptr != pModel)
  {
    const QString sName =
        pModel->data(pProxyModel->mapToSource(current), Qt::DisplayRole, resource_item::c_iColumnName).toString();
    m_spUi->pResourceLineEdit->setText(sName);
    on_pResourceLineEdit_editingFinished();
  }
}
