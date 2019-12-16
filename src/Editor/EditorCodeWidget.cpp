#include "EditorCodeWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "Backend/Scene.h"
#include "Backend/ScriptRunner.h"
#include "Backend/ScriptRunnerSignalEmiter.h"
#include "Scenes/SceneMainScreen.h"
#include "Script/BackgroundSnippetOverlay.h"
#include "Script/IconSnippetOverlay.h"
#include "Script/ResourceSnippetOverlay.h"
#include "Script/ScriptHighlighter.h"
#include "Script/TextSnippetOverlay.h"
#include "Script/TimerSnippetOverlay.h"
#include "ui_EditorCodeWidget.h"
#include "ui_EditorActionBar.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

namespace
{
  const char c_sIdProperty[] = "ID";
}

//----------------------------------------------------------------------------------------
//
CEditorCodeWidget::CEditorCodeWidget(QWidget* pParent) :
  CEditorWidgetBase(pParent),
  m_spUi(new Ui::CEditorCodeWidget),
  m_spBackgroundSnippetOverlay(std::make_unique<CBackgroundSnippetOverlay>()),
  m_spIconSnippetOverlay(std::make_unique<CIconSnippetOverlay>()),
  m_spResourceSnippetOverlay(std::make_unique<CResourceSnippetOverlay>()),
  m_spTextSnippetOverlay(std::make_unique<CTextSnippetOverlay>()),
  m_spTimerSnippetOverlay(std::make_unique<CTimerSnippetOverlay>()),
  m_spSettings(CApplication::Instance()->Settings()),
  m_spCurrentProject(nullptr),
  m_wpDbManager(),
  m_wpScriptRunner(),
  m_cachedScriptsMap(),
  m_iLastIndex(-1)
{
  m_spUi->setupUi(this);
  m_spUi->pSceneView->setVisible(false);
  m_pHighlighter = new CScriptHighlighter(m_spUi->pCodeEdit->document());
}

CEditorCodeWidget::~CEditorCodeWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();
  m_wpScriptRunner = CApplication::Instance()->System<CScriptRunner>();

  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    connect(spDbManager.get(), &CDatabaseManager::SignalSceneAdded,
            this, &CEditorCodeWidget::SlotSceneAdded);
    connect(spDbManager.get(), &CDatabaseManager::SignalSceneRemoved,
            this, &CEditorCodeWidget::SlotSceneRemoved);
    connect(spDbManager.get(), &CDatabaseManager::SignalSceneRenamed,
            this, &CEditorCodeWidget::SlotSceneRenamed);
  }

  m_spBackgroundSnippetOverlay->Initialize(ResourceModel());
  m_spIconSnippetOverlay->Initialize(ResourceModel());
  m_spResourceSnippetOverlay->Initialize(ResourceModel());

  m_spBackgroundSnippetOverlay->Hide();
  m_spIconSnippetOverlay->Hide();
  m_spResourceSnippetOverlay->Hide();
  m_spTextSnippetOverlay->Hide();
  m_spTimerSnippetOverlay->Hide();

  connect(m_spBackgroundSnippetOverlay.get(), &CBackgroundSnippetOverlay::SignalBackgroundCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spIconSnippetOverlay.get(), &CIconSnippetOverlay::SignalIconCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spResourceSnippetOverlay.get(), &CResourceSnippetOverlay::SignalResourceCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spTextSnippetOverlay.get(), &CTextSnippetOverlay::SignalTextSnippetCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);
  connect(m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::SignalTimerCode,
          this, &CEditorCodeWidget::SlotInsertGeneratedCode);

  m_iLastIndex = m_spUi->pSceneComboBox->currentIndex();

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::LoadProject(tspProject spProject)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == spProject) { return; }

  m_spCurrentProject = spProject;
  m_spCurrentProject->m_rwLock.lockForRead();
  auto vspScenes = m_spCurrentProject->m_vspScenes;
  m_spCurrentProject->m_rwLock.unlock();

  for (auto spScene : vspScenes)
  {
    qint32 iId = -1;
    QString sName;
    if (nullptr != spScene)
    {
      QReadLocker locker(&spScene->m_rwLock);
      sName = spScene->m_sName;
      iId = spScene->m_iId;
    }

    if (-1 != iId)
    {
      m_spUi->pSceneComboBox->blockSignals(true);
      m_spUi->pSceneComboBox->addItem(sName, iId);
      m_spUi->pSceneComboBox->blockSignals(false);
    }
  }

  if (m_spUi->pSceneComboBox->count() > 0)
  {
    m_spUi->pSceneComboBox->blockSignals(true);
    m_spUi->pSceneComboBox->setCurrentIndex(0);
    m_spUi->pSceneComboBox->blockSignals(false);
    on_pSceneComboBox_currentIndexChanged(0);
  }

  m_spResourceSnippetOverlay->LoadProject(m_spCurrentProject);

  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiter = spScriptRunner->SignalEmmitter();
    connect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalExecutionError,
            m_spUi->pCodeEdit, &CScriptEditorWidget::SlotExecutionError, Qt::QueuedConnection);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  SlotDebugStop();
  m_spUi->pCodeEdit->ResetWidget();

  auto spScriptRunner = m_wpScriptRunner.lock();
  if (nullptr != spScriptRunner)
  {
    auto spSignalEmmiter = spScriptRunner->SignalEmmitter();
    disconnect(spSignalEmmiter.get(), &CScriptRunnerSignalEmiter::SignalExecutionError,
            m_spUi->pCodeEdit, &CScriptEditorWidget::SlotExecutionError);
  }

  m_spResourceSnippetOverlay->UnloadProject();

  m_spCurrentProject = nullptr;

  m_spUi->pSceneComboBox->clear();
  m_cachedScriptsMap.clear();

  m_spBackgroundSnippetOverlay->Hide();
  m_spIconSnippetOverlay->Hide();
  m_spResourceSnippetOverlay->Hide();
  m_spTextSnippetOverlay->Hide();
  m_spTimerSnippetOverlay->Hide();
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SaveProject()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  const QString sProjectName = m_spCurrentProject->m_sName;
  m_spCurrentProject->m_rwLock.unlock();

  // save current contents
  qint32 iId = m_spUi->pSceneComboBox->itemData(m_spUi->pSceneComboBox->currentIndex()).toInt();
  auto itMap = m_cachedScriptsMap.find(iId);
  if (m_cachedScriptsMap.end() != itMap)
  {
    itMap->second.m_data = m_spUi->pCodeEdit->toPlainText().toUtf8();
  }

  // iterate through and save content
  for (auto it = m_cachedScriptsMap.begin(); m_cachedScriptsMap.end() != it; ++it)
  {
    if (it->second.m_bChanged)
    {
      SetSceneScriptModifiedFlag(it->first, false);

      auto spDbManager = m_wpDbManager.lock();
      if (nullptr != spDbManager)
      {
        auto spScene = spDbManager->FindScene(m_spCurrentProject, it->first);
        if (nullptr != spScene)
        {
          QReadLocker lockerScene(&spScene->m_rwLock);
          QString sScriptName = spScene->m_sScript;

          auto spResource = spDbManager->FindResource(m_spCurrentProject, sScriptName);
          if (nullptr != spResource)
          {
            QReadLocker lockerResource(&spResource->m_rwLock);
            QUrl sPath = spResource->m_sPath;
            const QString sFilePath = ResourceUrlToAbsolutePath(sPath, sProjectName);

            QFile file(sFilePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
              it->second.m_bIgnoreNextModification = true;
              file.write(it->second.m_data);
            }
            else
            {
              qWarning() << "Registered script resource could not be opened.";
            }
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::LoadResource(tspResource spResource)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  spResource->m_rwLock.lockForRead();
  const QString sName = spResource->m_sName;
  spResource->m_rwLock.unlock();

  qint32 iId = -1;
  QReadLocker projectLocker(&m_spCurrentProject->m_rwLock);
  for (tspScene spScene : m_spCurrentProject->m_vspScenes)
  {
    QReadLocker sceneLocker(&spScene->m_rwLock);
    if (spScene->m_sScript == sName)
    {
      iId = spScene->m_iId;
      break;
    }
  }
  projectLocker.unlock();

  if (-1 != iId)
  {
    qint32 iIndex = m_spUi->pSceneComboBox->findData(iId);
    m_spUi->pSceneComboBox->setCurrentIndex(iIndex);
    on_pSceneComboBox_currentIndexChanged(iIndex);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    disconnect(ActionBar()->m_spUi->pDebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStart);
    disconnect(ActionBar()->m_spUi->pStopDebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStop);
    disconnect(ActionBar()->m_spUi->pAddShowBackgroundCode, &QPushButton::clicked,
            m_spBackgroundSnippetOverlay.get(), &CBackgroundSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->pAddShowIconCode, &QPushButton::clicked,
            m_spIconSnippetOverlay.get(), &CIconSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->pAddShowImageCode, &QPushButton::clicked,
            m_spResourceSnippetOverlay.get(), &CResourceSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->pAddTextCode, &QPushButton::clicked,
            m_spTextSnippetOverlay.get(), &CTextSnippetOverlay::Show);
    disconnect(ActionBar()->m_spUi->pAddTimerCode, &QPushButton::clicked,
            m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::Show);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowCodeActionBar();
    connect(ActionBar()->m_spUi->pDebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStart);
    connect(ActionBar()->m_spUi->pStopDebugButton, &QPushButton::clicked,
            this, &CEditorCodeWidget::SlotDebugStop);
    connect(ActionBar()->m_spUi->pAddShowBackgroundCode, &QPushButton::clicked,
            m_spBackgroundSnippetOverlay.get(), &CBackgroundSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->pAddShowIconCode, &QPushButton::clicked,
            m_spIconSnippetOverlay.get(), &CIconSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->pAddShowImageCode, &QPushButton::clicked,
            m_spResourceSnippetOverlay.get(), &CResourceSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->pAddTextCode, &QPushButton::clicked,
            m_spTextSnippetOverlay.get(), &CTextSnippetOverlay::Show);
    connect(ActionBar()->m_spUi->pAddTimerCode, &QPushButton::clicked,
            m_spTimerSnippetOverlay.get(), &CTimerSnippetOverlay::Show);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::on_pSceneComboBox_currentIndexChanged(qint32 iIndex)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  const QString sProjectName = m_spCurrentProject->m_sName;
  m_spCurrentProject->m_rwLock.unlock();

  // save old contents
  qint32 iId = m_spUi->pSceneComboBox->itemData(m_iLastIndex).toInt();
  auto itMap = m_cachedScriptsMap.find(iId);
  if (m_cachedScriptsMap.end() != itMap)
  {
    itMap->second.m_data = m_spUi->pCodeEdit->toPlainText().toUtf8();
  }

  // handle new scene content
  iId = m_spUi->pSceneComboBox->itemData(iIndex).toInt();
  itMap = m_cachedScriptsMap.find(iId);
  if (m_cachedScriptsMap.end() != itMap)
  {
    m_spUi->pCodeEdit->blockSignals(true);
    m_spUi->pCodeEdit->clear();
    m_spUi->pCodeEdit->setPlainText(QString::fromUtf8(m_cachedScriptsMap[iId].m_data));
    m_spUi->pCodeEdit->blockSignals(false);
  }
  else
  {
    auto spDbManager = m_wpDbManager.lock();
    if (nullptr != spDbManager)
    {
      auto spScene = spDbManager->FindScene(m_spCurrentProject, iId);
      if (nullptr != spScene)
      {
        QReadLocker lockerScene(&spScene->m_rwLock);
        QString sScriptName = spScene->m_sScript;

        // create new script ptompt
        if (sScriptName.isNull() || sScriptName.isEmpty())
        {
          AddNewScriptFile(spScene);
          sScriptName = spScene->m_sScript;
        }

        auto spResource = spDbManager->FindResource(m_spCurrentProject, sScriptName);
        if (nullptr != spResource)
        {
          QReadLocker lockerResource(&spResource->m_rwLock);
          QUrl sPath = spResource->m_sPath;
          const QString sFilePath = ResourceUrlToAbsolutePath(sPath, sProjectName);

          if (QFileInfo(sFilePath).exists())
          {
            m_cachedScriptsMap.insert({iId, SCachedMapItem()});
            m_cachedScriptsMap[iId].m_data = LoadScriptFile(sFilePath);
            m_cachedScriptsMap[iId].m_watcher.removePaths(m_cachedScriptsMap[iId].m_watcher.files());
            m_cachedScriptsMap[iId].m_watcher.removePaths(m_cachedScriptsMap[iId].m_watcher.directories());
            m_cachedScriptsMap[iId].m_watcher.addPath(QFileInfo(sFilePath).absoluteFilePath());
            m_cachedScriptsMap[iId].m_watcher.setProperty(c_sIdProperty, iId);
            connect(&m_cachedScriptsMap[iId].m_watcher, &QFileSystemWatcher::fileChanged,
                    this, &CEditorCodeWidget::SlotFileChanged, Qt::UniqueConnection);

            m_spUi->pCodeEdit->blockSignals(true);
            m_spUi->pCodeEdit->clear();
            m_spUi->pCodeEdit->setPlainText(QString::fromUtf8(m_cachedScriptsMap[iId].m_data));
            m_spUi->pCodeEdit->blockSignals(false);
          }
          else
          {
            qWarning() << "Registered script resource does not exist.";
          }
        }
      }
    }
  }

  m_spUi->pCodeEdit->ResetWidget();
  m_spUi->pCodeEdit->update();

  m_iLastIndex = m_spUi->pSceneComboBox->currentIndex();
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::on_pCodeEdit_textChanged()
{
  WIDGET_INITIALIZED_GUARD

  qint32 iId = m_spUi->pSceneComboBox->itemData(m_spUi->pSceneComboBox->currentIndex()).toInt();
  SetSceneScriptModifiedFlag(iId, true);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotDebugStart()
{
  WIDGET_INITIALIZED_GUARD

  m_spUi->pSceneView->setVisible(true);
  QLayout* pLayout = m_spUi->pSceneView->layout();
  if (nullptr != pLayout)
  {
    if (nullptr != ActionBar())
    {
      ActionBar()->m_spUi->pDebugButton->hide();
      ActionBar()->m_spUi->pStopDebugButton->show();
    }

    // get Scene name
    auto spDbManager = m_wpDbManager.lock();
    QString sSceneName = QString();
    if (nullptr != spDbManager)
    {
      qint32 iId = m_spUi->pSceneComboBox->itemData(m_iLastIndex).toInt();
      auto spScene = spDbManager->FindScene(m_spCurrentProject, iId);
      if (nullptr != spScene)
      {
        QReadLocker locker(&spScene->m_rwLock);
        sSceneName = spScene->m_sName;
      }
    }

    m_spCurrentProject->m_rwLock.lockForRead();
    qint32 iCurrProject = m_spCurrentProject->m_iId;
    m_spCurrentProject->m_rwLock.unlock();
    CSceneMainScreen* pMainSceneScreen = new CSceneMainScreen(m_spUi->pSceneView);
    pMainSceneScreen->LoadProject(iCurrProject, sSceneName);

    pLayout->addWidget(pMainSceneScreen);

    m_spUi->pCodeEdit->ResetWidget();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotDebugStop()
{
  WIDGET_INITIALIZED_GUARD

  QLayout* pLayout = m_spUi->pSceneView->layout();
  if (nullptr != pLayout)
  {
    if (nullptr != ActionBar())
    {
      ActionBar()->m_spUi->pDebugButton->show();
      ActionBar()->m_spUi->pStopDebugButton->hide();
    }

    auto pItem = pLayout->takeAt(0);
    if (nullptr != pItem)
    {
      CSceneMainScreen* pMainSceneScreen =
          qobject_cast<CSceneMainScreen*>(pItem->widget());
      pMainSceneScreen->SlotQuit();
      delete pMainSceneScreen;
    }
  }
  m_spUi->pSceneView->setVisible(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotFileChanged(const QString& sPath)
{
  WIDGET_INITIALIZED_GUARD

  QFileSystemWatcher* pWatcher = qobject_cast<QFileSystemWatcher*>(sender());
  if (nullptr != pWatcher)
  {
    qint32 iId = pWatcher->property(c_sIdProperty).toInt();
    auto it = m_cachedScriptsMap.find(iId);
    if (m_cachedScriptsMap.end() != it)
    {
      if (it->second.m_bIgnoreNextModification)
      {
        it->second.m_bIgnoreNextModification = false;
      }
      else
      {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified on the disc.");
        msgBox.setInformativeText("Do you want to reload the file or keep the local file?");
        QPushButton* pReloadButton = msgBox.addButton(tr("Reload"), QMessageBox::AcceptRole);
        QPushButton* pKeepButton = msgBox.addButton(tr("Keep"), QMessageBox::RejectRole);
        msgBox.setDefaultButton(pKeepButton);
        msgBox.exec();

        if (msgBox.clickedButton() == pReloadButton)
        {
          it->second.m_data = LoadScriptFile(sPath);
          m_spUi->pCodeEdit->blockSignals(true);
          m_spUi->pCodeEdit->clear();
          m_spUi->pCodeEdit->setPlainText(QString::fromUtf8(m_cachedScriptsMap[iId].m_data));
          m_spUi->pCodeEdit->blockSignals(false);
          SetSceneScriptModifiedFlag(iId, false);
        }
        else
        {
          SetSceneScriptModifiedFlag(iId, true);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotInsertGeneratedCode(const QString& sCode)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spUi->pCodeEdit->insertPlainText(sCode);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotSceneAdded(qint32 iProjId, qint32 iId)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spCurrentProject->m_iId;
  m_spCurrentProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  QString sName = FindSceneName(iId);
  m_spUi->pSceneComboBox->blockSignals(true);
  m_spUi->pSceneComboBox->addItem(sName, iId);
  m_spUi->pSceneComboBox->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotSceneRenamed(qint32 iProjId, qint32 iId)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spCurrentProject->m_iId;
  m_spCurrentProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  QString sName = FindSceneName(iId);
  qint32 iIndex = m_spUi->pSceneComboBox->findData(iId);
  m_spUi->pSceneComboBox->setItemText(iIndex, sName);
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SlotSceneRemoved(qint32 iProjId, qint32 iId)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spCurrentProject->m_rwLock.lockForRead();
  qint32 iCurrentId = m_spCurrentProject->m_iId;
  m_spCurrentProject->m_rwLock.unlock();

  if (iCurrentId != iProjId) { return; }

  qint32 iIndex = m_spUi->pSceneComboBox->findData(iId);
  m_spUi->pSceneComboBox->removeItem(iIndex);

  // delete cached script
  auto it = m_cachedScriptsMap.find(iId);
  if (m_cachedScriptsMap.end() != it)
  {
    m_cachedScriptsMap.erase(it);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::AddNewScriptFile(tspScene spScene)
{
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spScene && nullptr != spDbManager)
  {
    // if there is no script -> create
    QReadLocker locker(&spScene->m_rwLock);
    if (spScene->m_sScript.isNull() || spScene->m_sScript.isEmpty())
    {
      const QString sName = PhysicalProjectName(m_spCurrentProject);
      QString sCurrentFolder = CApplication::Instance()->Settings()->ContentFolder();
      QUrl sUrl = QFileDialog::getSaveFileUrl(this,
          tr("Create Script File"), QUrl::fromLocalFile(sCurrentFolder + "/" + sName),
          "Script Files (*.js)");

      if (sUrl.isValid())
      {
        QFileInfo info(sUrl.toLocalFile());
        QDir projectDir(m_spSettings->ContentFolder() + "/" + sName);
        if (!info.absoluteFilePath().contains(projectDir.absolutePath()))
        {
          qWarning() << "File is not in subfolder of Project.";
        }
        else
        {
          QString sRelativePath = projectDir.relativeFilePath(info.absoluteFilePath());
          QUrl sUrlToSave = QUrl::fromLocalFile(sRelativePath);
          QFile jsFile(info.absoluteFilePath());
          if (jsFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
          {
            jsFile.write(QString("// instert code to control scene").toUtf8());
            QString sResource = spDbManager->AddResource(m_spCurrentProject, sUrlToSave,
                                                         EResourceType::eOther);
            spScene->m_sScript = sResource;
          }
          else
          {
            qWarning() << "Could not write script file.";
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CEditorCodeWidget::FindSceneName(qint32 iId)
{
  QString sName;
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    auto spScene = spDbManager->FindScene(m_spCurrentProject, iId);
    if (nullptr != spScene)
    {
      QReadLocker locker(&spScene->m_rwLock);
      sName = spScene->m_sName;
    }
  }
  return sName;
}

//----------------------------------------------------------------------------------------
//
QByteArray CEditorCodeWidget::LoadScriptFile(const QString& sFile)
{
  QByteArray scriptContents;
  QFile scriptFile(sFile);
  if (scriptFile.open(QIODevice::ReadOnly))
  {
    scriptContents = scriptFile.readAll();
  }
  else
  {
    qWarning() << "Registered script resource could not be opened.";
  }
  return scriptContents;
}

//----------------------------------------------------------------------------------------
//
void CEditorCodeWidget::SetSceneScriptModifiedFlag(qint32 iId, bool bModified)
{
  auto it = m_cachedScriptsMap.find(iId);
  if (m_cachedScriptsMap.end() != it && it->second.m_bChanged == bModified) return;
  if (m_cachedScriptsMap.end() != it)
  {
    it->second.m_bChanged = bModified;
  }

  QString sName = FindSceneName(iId);
  qint32 iIndex = m_spUi->pSceneComboBox->findData(iId);
  if (!bModified)
  {
    m_spUi->pSceneComboBox->setItemText(iIndex, sName);
  }
  else
  {
    m_spUi->pSceneComboBox->setItemText(iIndex, sName + " *");
    emit SignalProjectEdited();
  }
}
