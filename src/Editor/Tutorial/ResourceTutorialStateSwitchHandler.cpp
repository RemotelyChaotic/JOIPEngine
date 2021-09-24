#include "ResourceTutorialStateSwitchHandler.h"
#include "Application.h"
#include "Editor/EditorModel.h"
#include "Editor/EditorResourceWidget.h"
#include "Editor/Resources/ResourceTreeItem.h"
#include "Editor/Resources/ResourceTreeItemModel.h"
#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>

CResourceTutorialStateSwitchHandler::
CResourceTutorialStateSwitchHandler(QPointer<CEditorResourceWidget> pParentWidget,
                                    const std::shared_ptr<Ui::CEditorResourceWidget>& spUi) :
  QObject(nullptr),
  ITutorialStateSwitchHandler(),
  m_pParentWidget(pParentWidget),
  m_spUi(spUi),
  m_currentState(ETutorialState::eFinished),
  m_connection()
{

}

CResourceTutorialStateSwitchHandler::~CResourceTutorialStateSwitchHandler()
{

}

//----------------------------------------------------------------------------------------
//
void CResourceTutorialStateSwitchHandler::OnResetStates()
{
  m_currentState = ETutorialState::eFinished;
  if (m_connection)
  {
    disconnect(m_connection);
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTutorialStateSwitchHandler::OnStateSwitch(ETutorialState newState,
                                                        ETutorialState oldstate)
{
  Q_UNUSED(oldstate)
  m_currentState = newState;

  if (m_connection)
  {
    disconnect(m_connection);
  }

  switch (newState)
  {
    case ETutorialState::eResourcePanel:
    {
      QItemSelectionModel* pSelectionModel = m_spUi->pResourceModelView->CurrentSelectionModel();
      m_connection =
          connect(pSelectionModel, &QItemSelectionModel::currentChanged,
                  this, &CResourceTutorialStateSwitchHandler::SlotCurrentChanged);
    } break;
    default: break;
  }
}

//----------------------------------------------------------------------------------------
//
void CResourceTutorialStateSwitchHandler::SlotCurrentChanged(const QModelIndex& current,
                                                             const QModelIndex& previous)
{
  if (ETutorialState::eResourcePanel == m_currentState._to_integral())
  {
    auto spCurrentProject = m_pParentWidget->EditorModel()->CurrentProject();
    if (nullptr == spCurrentProject) { return; }
    Q_UNUSED(previous);

    QSortFilterProxyModel* pProxyModel =
      m_spUi->pResourceModelView->Proxy().data();
    CResourceTreeItemModel* pModel =
      dynamic_cast<CResourceTreeItemModel*>(pProxyModel->sourceModel());

    if (nullptr != pModel)
    {
      const QString sName =
          pModel->data(pProxyModel->mapToSource(current), Qt::DisplayRole, resource_item::c_iColumnName).toString();

      auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
      if (nullptr != spDbManager)
      {
        auto spResource = spDbManager->FindResourceInProject(spCurrentProject, sName);
        if (nullptr != spResource)
        {
          QReadLocker locker(&spResource->m_rwLock);
          if (EResourceType::eImage == spResource->m_type._to_integral())
          {
            m_pParentWidget->EditorModel()->NextTutorialState();
          }
        }
      }
    }
  }
}
