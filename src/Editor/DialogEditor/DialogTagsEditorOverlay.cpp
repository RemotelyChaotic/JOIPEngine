#include "DialogTagsEditorOverlay.h"
#include "Application.h"
#include "CommandAddRemoveDialogTags.h"

#include "ui_DialogTagsEditorOverlay.h"

#include "Editor/CommandChangeTag.h"
#include "Editor/DialogEditor/DialogEditorTreeModel.h"

#include "Systems/DatabaseManager.h"

#include "Widgets/TagCompleter.h"

#include <QStandardItemModel>
#include <QUndoStack>

namespace
{
  const char c_sTagNameProperty[] = "TagName";
}

CDialogueTagsEditorOverlay::CDialogueTagsEditorOverlay(QWidget* pParent) :
    COverlayBase(0, pParent),
    m_spUi(std::make_unique<Ui::CDialogueTagsEditorOverlay>()),
    m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spUi->setupUi(this);

  m_spUi->pTagsFrame->SetCallbacks(std::bind(&CDialogueTagsEditorOverlay::TagAdded, this,
                                             std::placeholders::_1, std::placeholders::_2),
                                   std::bind(&CDialogueTagsEditorOverlay::TagRemoved, this,
                                             std::placeholders::_1));
  m_spUi->pTagsFrame->SetSortFunction(std::bind(&CDialogueTagsEditorOverlay::SortTags, this,
                                                std::placeholders::_1));

  m_pCompleterModel = new QStandardItemModel(this);

  m_pCompleter = new CTagCompleter(m_pCompleterModel, m_spUi->pLineEdit);
  m_pCompleter->setFilterMode(Qt::MatchContains);
  m_pCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
  m_pCompleter->setCompletionRole(Qt::DisplayRole);
  m_pCompleter->setCompletionColumn(0);
  m_pCompleter->setMaxVisibleItems(10);
  m_spUi->pLineEdit->setCompleter(m_pCompleter);
}

CDialogueTagsEditorOverlay::~CDialogueTagsEditorOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::SetPath(const QStringList& vsPath)
{
  m_vsPath = vsPath;
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::SetProject(const tspProject& spCurrentProject)
{
  m_spCurrentProject = spCurrentProject;
  m_pCompleter->SetCurrentProject(m_spCurrentProject);
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::SetUndoStack(QPointer<QUndoStack> pUndoStack)
{
  m_pUndoStack = pUndoStack;
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::SetModel(QPointer<CDialogueEditorTreeModel> pModel)
{
  m_pModel = pModel;
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::Hide()
{
  /*
  if (auto spDbManager = m_wpDbManager.lock())
  {
    disconnect(spDbManager.get(), &CDatabaseManager::SignalTagAdded,
               this, &CDialogueTagsEditorOverlay::SlotTagAdded);
    disconnect(spDbManager.get(), &CDatabaseManager::SignalTagRemoved,
               this, &CDialogueTagsEditorOverlay::SlotTagRemoved);
  }
*/

  COverlayBase::Hide();
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::Show()
{
  Initialize();

  /*
  if (auto spDbManager = m_wpDbManager.lock())
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalTagAdded,
            this, &CDialogueTagsEditorOverlay::SlotTagAdded, Qt::UniqueConnection);
    connect(spDbManager.get(), &CDatabaseManager::SignalTagRemoved,
            this, &CDialogueTagsEditorOverlay::SlotTagRemoved, Qt::UniqueConnection);
  }
  */

  COverlayBase::Show();
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::on_pLineEdit_editingFinished()
{
  const QString& sTagName = m_spUi->pLineEdit->text();
  const QString& sTagDescription = m_spUi->pDescriptionLineEdit->text();
  if (auto spDbManager = m_wpDbManager.lock();
      nullptr != m_pUndoStack && nullptr != spDbManager && nullptr != m_spCurrentProject &&
      !sTagName.isEmpty())
  {
    auto spNode = std::dynamic_pointer_cast<CDialogueNodeDialogue>(m_pModel->Node(m_pModel->Index(m_vsPath)));
    if (nullptr != spNode)
    {
      QString sProject;
      tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, spNode->m_sFileId);
      tspTag spTag = spDbManager->FindTagInProject(m_spCurrentProject, sTagName);
      {
        QReadLocker locker(&m_spCurrentProject->m_rwLock);
        sProject = m_spCurrentProject->m_sName;
      }

      auto fnTagTypeFromName = [](const QString& sName, const QString& sTagDefault) -> QString {
        if (sName.contains(":"))
        {
          return sName.left(sName.indexOf(":"));
        }
        return sTagDefault;
      };

      if (nullptr == spTag)
      {
        spTag = std::make_unique<STag>(QString(), sTagName, sTagDescription);
        if (nullptr != spResource)
        {
          {
            QReadLocker resLocker(&spResource->m_rwLock);
            spTag->m_sType = fnTagTypeFromName(sTagName, spResource->m_type._to_string());
          }
          m_pUndoStack->push(new CCommandAddDialogueTag(sProject, m_vsPath, spNode, spTag, m_pModel));
          SlotTagAdded(sTagName);
        }
      }
      else
      {
        QString sOldDescription;
        QString sOldType;
        QString sNewType;
        bool bTagFound = false;
        {
          QReadLocker resLocker(&spResource->m_rwLock);
          QReadLocker tagLocker(&spTag->m_rwLock);
          bTagFound =
              spNode->m_tags.end() !=
              spNode->m_tags.find(sTagName);
          sOldDescription = spTag->m_sDescribtion;
          sOldType = spTag->m_sType;
          sNewType = fnTagTypeFromName(sTagName, sOldType);
        }
        if ((sOldDescription != sTagDescription && !sTagDescription.isEmpty()) ||
            (sOldType != sNewType && !sNewType.isEmpty()))
        {
          m_pUndoStack->push(new CCommandChangeTag(sProject, sTagName, sNewType, sOldType,
                                                   sTagDescription, sOldDescription));
          m_spUi->pTagsFrame->UpdateToolTip(sTagName, sTagDescription);
          emit SignalTagsChanged();
        }
        if (!bTagFound)
        {
          m_pUndoStack->push(new CCommandAddDialogueTag(sProject, m_vsPath, spNode, spTag, m_pModel));
          SlotTagAdded(sTagName);
        }
      }
    }
  }

  QSignalBlocker b(m_spUi->pDescriptionLineEdit);
  m_spUi->pDescriptionLineEdit->setText(QString());
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::on_pDescriptionLineEdit_editingFinished()
{
  on_pLineEdit_editingFinished();
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::on_pConfirmButton_clicked()
{
  m_spUi->pTagsFrame->ClearTags();
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::SlotRemoveTagClicked()
{
  QString sTagName = sender()->property(c_sTagNameProperty).toString();
  if (auto spDbManager = m_wpDbManager.lock();
      nullptr != m_pUndoStack && nullptr != spDbManager && nullptr != m_spCurrentProject &&
      nullptr != m_pModel)
  {
    auto spNode = m_pModel->Node(m_pModel->Index(m_vsPath));

    QString sProject;
    tspTag spTag = spDbManager->FindTagInProject(m_spCurrentProject, sTagName);
    {
      QReadLocker locker(&m_spCurrentProject->m_rwLock);
      sProject = m_spCurrentProject->m_sName;
    }
    if (nullptr != spTag)
    {
      m_pUndoStack->push(new CCommandRemoveDialogueTag(sProject, m_vsPath, spNode, spTag, m_pModel));
      SlotTagRemoved(sTagName);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::SlotTagAdded(const QString& sName)
{
  if (nullptr != m_pModel)
  {
    if (auto spDbManager = m_wpDbManager.lock();
        nullptr != spDbManager && nullptr != m_spCurrentProject)
    {
      tspTag spTag = spDbManager->FindTagInProject(m_spCurrentProject, sName);

      m_pCompleterModel->appendRow(new QStandardItem(sName));
      m_spUi->pTagsFrame->AddTags({spTag});
    }
  }
}

//----------------------------------------------------------------------------------------
//

void CDialogueTagsEditorOverlay::SlotTagRemoved(const QString& sName)
{
  QList<QStandardItem*> vpItems = m_pCompleterModel->findItems(sName);
  if (vpItems.size() > 0)
  {
    QModelIndex idx = m_pCompleterModel->indexFromItem(vpItems[0]);
    m_pCompleterModel->removeRows(idx.row(), 1);
  }
  m_spUi->pTagsFrame->RemoveTags({sName});
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::Initialize()
{
  m_spUi->pTagsFrame->ClearTags();
  m_pCompleterModel->clear();

  if (nullptr != m_spCurrentProject && nullptr != m_pModel)
  {
    auto spNode = std::dynamic_pointer_cast<CDialogueNodeDialogue>(
        m_pModel->Node(m_pModel->Index(m_vsPath)));
    if (nullptr != spNode)
    {
      tspTagMap vsTags = spNode->m_tags;

      std::vector<std::shared_ptr<SLockableTagData>> vspTagsToAdd;
      {
        QReadLocker pLocker(&m_spCurrentProject->m_rwLock);
        for (const auto& [sName, spTag] : m_spCurrentProject->m_vspTags)
        {
          m_pCompleterModel->appendRow(new QStandardItem(sName));
          if (vsTags.find(sName) != vsTags.end())
          {
            vspTagsToAdd.push_back(spTag);
          }
        }
      }

      m_spUi->pTagsFrame->AddTags(vspTagsToAdd);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::SortTags(std::vector<std::shared_ptr<SLockableTagData>>& vspTags)
{
  std::sort(vspTags.begin(), vspTags.end(),
            [](const std::shared_ptr<SLockableTagData>& left,
               const std::shared_ptr<SLockableTagData>& right) {
              QReadLocker lockerLeft(&left->m_rwLock);
              QReadLocker lockerRight(&right->m_rwLock);
              return left->m_sName < right->m_sName;
            });
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::TagAdded(QPushButton* pButton, const QString& sTag)
{
  if (nullptr != pButton)
  {
    pButton->setProperty(c_sTagNameProperty, sTag);
    connect(pButton, &QPushButton::clicked,
            this, &CDialogueTagsEditorOverlay::SlotRemoveTagClicked);
  }
  emit SignalTagsChanged();
}

//----------------------------------------------------------------------------------------
//
void CDialogueTagsEditorOverlay::TagRemoved(const QStringList&)
{
  emit SignalTagsChanged();
}
