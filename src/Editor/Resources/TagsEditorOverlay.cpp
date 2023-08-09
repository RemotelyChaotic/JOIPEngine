#include "TagsEditorOverlay.h"
#include "Application.h"
#include "CommandChangeTags.h"
#include "TagCompleter.h"
#include "ui_TagsEditorOverlay.h"

#include "Systems/DatabaseManager.h"

#include <QStandardItemModel>
#include <QUndoStack>

namespace
{
  const char c_sTagNameProperty[] = "TagName";
}

CTagsEditorOverlay::CTagsEditorOverlay(QWidget* pParent) :
    COverlayBase(0, pParent),
    m_spUi(std::make_unique<Ui::CTagsEditorOverlay>()),
    m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
  m_spUi->setupUi(this);

  m_spUi->pTagsFrame->SetCallbacks(std::bind(&CTagsEditorOverlay::TagAdded, this,
                                             std::placeholders::_1, std::placeholders::_2),
                                   std::bind(&CTagsEditorOverlay::TagRemoved, this,
                                             std::placeholders::_1));
  m_spUi->pTagsFrame->SetSortFunction(std::bind(&CTagsEditorOverlay::SortTags, this,
                                                std::placeholders::_1));

  m_pCompleterModel = new QStandardItemModel(this);

  m_pCompleter = new CTagCompleter(m_pCompleterModel, m_spUi->pLineEdit);
  m_pCompleter->setFilterMode(Qt::MatchRecursive | Qt::MatchContains);
  m_pCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
  m_pCompleter->setCompletionRole(Qt::DisplayRole);
  m_pCompleter->setCompletionColumn(0);
  m_pCompleter->setMaxVisibleItems(10);
  m_spUi->pLineEdit->setCompleter(m_pCompleter);
}

CTagsEditorOverlay::~CTagsEditorOverlay()
{
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::SetProject(const tspProject& spCurrentProject)
{
  m_spCurrentProject = spCurrentProject;
  m_pCompleter->SetCurrentProject(m_spCurrentProject);
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::SetResource(const QString& sName)
{
  m_sResource = sName;
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::SetUndoStack(QPointer<QUndoStack> pUndoStack)
{
  m_pUndoStack = pUndoStack;
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorMainScreen", false);
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::Hide()
{
  if (auto spDbManager = m_wpDbManager.lock())
  {
    disconnect(spDbManager.get(), &CDatabaseManager::SignalTagAdded,
            this, &CTagsEditorOverlay::SlotTagAdded);
    disconnect(spDbManager.get(), &CDatabaseManager::SignalTagRemoved,
            this, &CTagsEditorOverlay::SlotTagRemoved);
  }

  COverlayBase::Hide();
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::Show()
{
  Initialize();

  if (auto spDbManager = m_wpDbManager.lock())
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalTagAdded,
            this, &CTagsEditorOverlay::SlotTagAdded, Qt::UniqueConnection);
    connect(spDbManager.get(), &CDatabaseManager::SignalTagRemoved,
            this, &CTagsEditorOverlay::SlotTagRemoved, Qt::UniqueConnection);
  }

  COverlayBase::Show();
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::Resize()
{
  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(width() / 2, height() / 2);

  move(newPos.x(), newPos.y());
  resize(width(), height());
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::on_pLineEdit_editingFinished()
{
  const QString& sTagName = m_spUi->pLineEdit->text();
  if (auto spDbManager = m_wpDbManager.lock();
      nullptr != m_pUndoStack && nullptr != spDbManager && nullptr != m_spCurrentProject &&
      !sTagName.isEmpty())
  {
    QString sProject;
    tspResource spResource = spDbManager->FindResourceInProject(m_spCurrentProject, m_sResource);
    tspTag spTag = spDbManager->FindTagInProject(m_spCurrentProject, sTagName);
    {
      QReadLocker locker(&m_spCurrentProject->m_rwLock);
      sProject = m_spCurrentProject->m_sName;
    }

    if (nullptr == spTag)
    {
      spTag = std::make_unique<STag>(QString(), sTagName, QString());
    }
    if (nullptr != spResource)
    {
      {
        QReadLocker resLocker(&spResource->m_rwLock);
        spTag->m_sType = spResource->m_type._to_string();
      }
      m_pUndoStack->push(new CCommandAddTag(sProject, m_sResource, spTag));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::on_pConfirmButton_clicked()
{
  m_spUi->pTagsFrame->ClearTags();
  Hide();
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::SlotRemoveTagClicked()
{
  QString sTagName = sender()->property(c_sTagNameProperty).toString();
  if (auto spDbManager = m_wpDbManager.lock();
      nullptr != m_pUndoStack && nullptr != spDbManager && nullptr != m_spCurrentProject)
  {
    QString sProject;
    tspTag spTag = spDbManager->FindTagInProject(m_spCurrentProject, sTagName);
    {
      QReadLocker locker(&m_spCurrentProject->m_rwLock);
      sProject = m_spCurrentProject->m_sName;
    }
    if (nullptr != spTag)
    {
      m_pUndoStack->push(new CCommandRemoveTag(sProject, m_sResource, spTag));
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::SlotTagAdded(qint32, const QString& sResource, const QString& sName)
{
  if (auto spDbManager = m_wpDbManager.lock();
      sResource == m_sResource && nullptr != spDbManager && nullptr != m_spCurrentProject)
  {
    tspTag spTag = spDbManager->FindTagInProject(m_spCurrentProject, sName);
    m_spUi->pTagsFrame->AddTags({spTag});
  }
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::SlotTagRemoved(qint32, const QString& sResource, const QString& sName)
{
  if (auto spDbManager = m_wpDbManager.lock();
      (sResource == m_sResource || sResource.isEmpty()) && nullptr != m_spCurrentProject)
  {
    m_spUi->pTagsFrame->RemoveTags({sName});
  }
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::Initialize()
{
  m_spUi->pTagsFrame->ClearTags();

  if (auto spDbManager = m_wpDbManager.lock();
      nullptr != m_spCurrentProject && nullptr != spDbManager)
  {
    auto spResource = spDbManager->FindResourceInProject(m_spCurrentProject, m_sResource);
    QReadLocker rLocker(&spResource->m_rwLock);
    tvsTags vsTags = spResource->m_vsResourceTags;
    rLocker.unlock();

    std::vector<std::shared_ptr<SLockableTagData>> vspTagsToAdd;
    {
      QReadLocker pLocker(&m_spCurrentProject->m_rwLock);
      for (const auto& [sName, spTag] : m_spCurrentProject->m_vspTags)
      {
        if (vsTags.find(sName) != vsTags.end())
        {
          vspTagsToAdd.push_back(spTag);
        }
      }
    }

    m_spUi->pTagsFrame->AddTags(vspTagsToAdd);
  }
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::SortTags(std::vector<std::shared_ptr<SLockableTagData>>& vspTags)
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
void CTagsEditorOverlay::TagAdded(QPushButton* pButton, const QString& sTag)
{
  if (nullptr != pButton)
  {
    pButton->setProperty(c_sTagNameProperty, sTag);
    connect(pButton, &QPushButton::clicked,
            this, &CTagsEditorOverlay::SlotRemoveTagClicked);
  }
  emit SignalTagsChanged();
}

//----------------------------------------------------------------------------------------
//
void CTagsEditorOverlay::TagRemoved(const QStringList&)
{
  emit SignalTagsChanged();
}
