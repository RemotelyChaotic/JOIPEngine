#include "EditorProjectSettingsWidget.h"
#include "Application.h"
#include "EditorActionBar.h"
#include "EditorModel.h"
#include "SVersion.h"
#include "out_Version.h"
#include "Project/KinkCompleter.h"
#include "Project/KinkSelectionOverlay.h"
#include "Project/KinkTreeModel.h"
#include "Systems/DatabaseManager.h"
#include "Systems/HelpFactory.h"
#include "Systems/Project.h"
#include "Widgets/FlowLayout.h"
#include "Widgets/HelpOverlay.h"
#include "ui_EditorProjectSettingsWidget.h"
#include "ui_EditorActionBar.h"

#include <QCompleter>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDebug>

namespace
{
  const char c_sKinkRootWidgetProperty[] = "KinkRootWidget";

  const QString c_sRenameProjectHelpId =    "Editor/RenameProject";
  const QString c_sProjectVersionHelpId =   "Editor/ProjectVersion";
  const QString c_sEngineVersionHelpId =    "Editor/EngineVersion";
  const QString c_sProjectDescribtionHelpId="Editor/ProjectDescribtion";
  const QString c_sFetishListHelpId =       "Editor/FetishList";
  const QString c_sAddFetishHelpId =        "Editor/AddFetish";
}

//----------------------------------------------------------------------------------------
//
CEditorProjectSettingsWidget::CEditorProjectSettingsWidget(QWidget *parent) :
  CEditorWidgetBase(parent),
  m_spUi(std::make_unique<Ui::CEditorProjectSettingsWidget>()),
  m_spKinkSelectionOverlay(std::make_unique<CKinkSelectionOverlay>(this)),
  m_vspKinks(),
  m_spCurrentProject(nullptr)
{
  m_spUi->setupUi(this);
}

CEditorProjectSettingsWidget::~CEditorProjectSettingsWidget()
{
  m_spUi->pFetishLineEdit->completer()->setModel(nullptr);
  m_spKinkSelectionOverlay.reset();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::Initialize()
{
  m_bInitialized = false;

  m_spKinkSelectionOverlay->Initialize(KinkModel());
  connect(KinkModel(), &CKinkTreeModel::SignalCheckedItem,
          this, &CEditorProjectSettingsWidget::SlotKinkChecked);

  auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
  if (nullptr != wpHelpFactory)
  {
    m_spUi->pTitleLineEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sRenameProjectHelpId);
    wpHelpFactory->RegisterHelp(c_sRenameProjectHelpId, ":/resources/help/editor/project_name_help.html");
    m_spUi->pProjectVersionContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sProjectVersionHelpId);
    wpHelpFactory->RegisterHelp(c_sProjectVersionHelpId, ":/resources/help/editor/projectsettings/projectversion_help.html");
    m_spUi->pEngineVersionContainer->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sEngineVersionHelpId);
    wpHelpFactory->RegisterHelp(c_sEngineVersionHelpId, ":/resources/help/editor/projectsettings/engineversion_help.html");
    m_spUi->pDescribtionTextEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sProjectDescribtionHelpId);
    wpHelpFactory->RegisterHelp(c_sProjectDescribtionHelpId, ":/resources/help/editor/projectsettings/describtion_textedit_help.html");
    m_spUi->pFetishListWidget->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sFetishListHelpId);
    wpHelpFactory->RegisterHelp(c_sFetishListHelpId, ":/resources/help/editor/projectsettings/fetish_tree_help.html");
    m_spUi->pFetishLineEdit->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddFetishHelpId);
    wpHelpFactory->RegisterHelp(c_sAddFetishHelpId, ":/resources/help/editor/addfetish_button_help.html");
    m_spUi->FetishOverlayButton->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sAddFetishHelpId);
    wpHelpFactory->RegisterHelp(c_sAddFetishHelpId, ":/resources/help/editor/addfetish_button_help.html");
  }

  auto spDataBaseManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr != spDataBaseManager)
  {
    connect(spDataBaseManager.get(), &CDatabaseManager::SignalProjectRenamed,
            this, &CEditorProjectSettingsWidget::SlotProjectRenamed, Qt::QueuedConnection);
  }

  CKinkCompleter* pCompleter = new CKinkCompleter(KinkModel(), m_spUi->pFetishLineEdit);
  pCompleter->setFilterMode(Qt::MatchRecursive | Qt::MatchContains);
  pCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  pCompleter->setCompletionMode(QCompleter::PopupCompletion);
  pCompleter->setCompletionRole(Qt::DisplayRole);
  pCompleter->setCompletionColumn(0);
  pCompleter->setMaxVisibleItems(10);
  m_spUi->pFetishLineEdit->setCompleter(pCompleter);

  CFlowLayout* pFlow = new CFlowLayout(m_spUi->pFetishListWidget, 9, 6, 6);
  m_spUi->pFetishListWidget->setLayout(pFlow);

  SVersion version(VERSION_XYZ);
  m_spUi->pEngineMajorVersion->setValue(version.m_iMajor);
  m_spUi->pEngineMinorVersion->setValue(version.m_iMinor);
  m_spUi->pEnginePatchVersion->setValue(version.m_iPatch);
  m_spUi->WarningIcon->setVisible(false);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::LoadProject(tspProject spProject)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr != m_spCurrentProject)
  {
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  m_spCurrentProject = spProject;
  if (nullptr != m_spCurrentProject)
  {
    QReadLocker locker(&m_spCurrentProject->m_rwLock);

    bool bReadOnly = EditorModel()->IsReadOnly();

    m_spUi->pTitleLineEdit->blockSignals(true);
    m_spUi->pTitleLineEdit->setText(m_spCurrentProject->m_sName);
    m_spUi->pTitleLineEdit->setReadOnly(bReadOnly);
    m_spUi->pTitleLineEdit->blockSignals(false);

    SVersion targetVersion(m_spCurrentProject->m_iTargetVersion);
    SVersion engineVersion(VERSION_XYZ);
    m_spUi->pEngineMajorVersion->setValue(static_cast<qint32>(targetVersion.m_iMajor));
    m_spUi->pEngineMinorVersion->setValue(static_cast<qint32>(targetVersion.m_iMinor));
    m_spUi->pEnginePatchVersion->setValue(static_cast<qint32>(targetVersion.m_iPatch));
    m_spUi->WarningIcon->setVisible(targetVersion != engineVersion);

    SVersion projVersion(m_spCurrentProject->m_iVersion);
    m_spUi->pProjectMajorVersion->blockSignals(true);
    m_spUi->pProjectMinorVersion->blockSignals(true);
    m_spUi->pProjectPatchVersion->blockSignals(true);
    m_spUi->pProjectMajorVersion->setValue(static_cast<qint32>(projVersion.m_iMajor));
    m_spUi->pProjectMinorVersion->setValue(static_cast<qint32>(projVersion.m_iMinor));
    m_spUi->pProjectPatchVersion->setValue(static_cast<qint32>(projVersion.m_iPatch));
    m_spUi->pProjectMajorVersion->setEnabled(!bReadOnly);
    m_spUi->pProjectMinorVersion->setEnabled(!bReadOnly);
    m_spUi->pProjectPatchVersion->setEnabled(!bReadOnly);
    m_spUi->pProjectMajorVersion->blockSignals(false);
    m_spUi->pProjectMinorVersion->blockSignals(false);
    m_spUi->pProjectPatchVersion->blockSignals(false);

    m_spUi->pDescribtionTextEdit->setPlainText(m_spCurrentProject->m_sDescribtion);
    m_spUi->pDescribtionTextEdit->setReadOnly(bReadOnly);

    m_spUi->pFetishLineEdit->setEnabled(!bReadOnly);
    m_spUi->pFetishListWidget->setEnabled(!bReadOnly);

    AddKinks(m_spCurrentProject->m_vsKinks);
    KinkModel()->SetSelections(m_spCurrentProject->m_vsKinks);

    m_spKinkSelectionOverlay->LoadProject(bReadOnly);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::UnloadProject()
{
  WIDGET_INITIALIZED_GUARD

  m_spCurrentProject = nullptr;

  m_spUi->pTitleLineEdit->setReadOnly(false);

  SVersion version(VERSION_XYZ);
  m_spUi->pEngineMajorVersion->setValue(version.m_iMajor);
  m_spUi->pEngineMinorVersion->setValue(version.m_iMinor);
  m_spUi->pEnginePatchVersion->setValue(version.m_iPatch);
  m_spUi->pProjectMajorVersion->setEnabled(true);
  m_spUi->pProjectMinorVersion->setEnabled(true);
  m_spUi->pProjectPatchVersion->setEnabled(true);
  m_spUi->WarningIcon->setVisible(false);

  m_spUi->pDescribtionTextEdit->clear();
  m_spUi->pDescribtionTextEdit->setReadOnly(false);

  m_spUi->pFetishLineEdit->setEnabled(true);
  m_spUi->pFetishListWidget->setEnabled(true);

  m_vspKinks.clear();
  KinkModel()->ResetSelections();

  m_spKinkSelectionOverlay->Hide();
  m_spKinkSelectionOverlay->UnloadProject();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SaveProject()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QWriteLocker locker(&m_spCurrentProject->m_rwLock);
  m_spCurrentProject->m_iVersion = SVersion(static_cast<quint32>(m_spUi->pProjectMajorVersion->value()),
                                            static_cast<quint32>(m_spUi->pProjectMinorVersion->value()),
                                            static_cast<quint32>(m_spUi->pProjectPatchVersion->value()));

  SVersion version(VERSION_XYZ);
  m_spUi->pEngineMajorVersion->setValue(version.m_iMajor);
  m_spUi->pEngineMinorVersion->setValue(version.m_iMinor);
  m_spUi->pEnginePatchVersion->setValue(version.m_iPatch);
  m_spCurrentProject->m_iTargetVersion = SVersion(static_cast<quint32>(m_spUi->pEngineMajorVersion->value()),
                                                  static_cast<quint32>(m_spUi->pEngineMinorVersion->value()),
                                                  static_cast<quint32>(m_spUi->pEnginePatchVersion->value()));
  m_spUi->WarningIcon->setVisible(false);

  m_spCurrentProject->m_sDescribtion = m_spUi->pDescribtionTextEdit->toPlainText();

  m_spCurrentProject->m_vsKinks.clear();
  for (qint32 i = 0; static_cast<qint32>(m_vspKinks.size()) > i; ++i)
  {
    QReadLocker locker(&m_vspKinks[static_cast<size_t>(i)]->m_rwLock);
    m_spCurrentProject->m_vsKinks.push_back(m_vspKinks[static_cast<size_t>(i)]->m_sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pTitleLineEdit_editingFinished()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QString sNewName = m_spUi->pTitleLineEdit->text();
  const QString sFinalName = EditorModel()->RenameProject(sNewName);
  m_spUi->pTitleLineEdit->blockSignals(true);
  m_spUi->pTitleLineEdit->setText(sFinalName);
  m_spUi->pTitleLineEdit->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pProjectMajorVersion_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pProjectMinorVersion_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pProjectPatchVersion_valueChanged(qint32 iValue)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  Q_UNUSED(iValue)
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pDescribtionTextEdit_textChanged()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_pFetishLineEdit_editingFinished()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  QString sKink = m_spUi->pFetishLineEdit->text();
  AddKinks({sKink});
  emit SignalProjectEdited();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::on_FetishOverlayButton_clicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  m_spKinkSelectionOverlay->Show();
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotKinkChecked(const QModelIndex& index, bool bChecked)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }
  QString sKink = KinkModel()->data(index, Qt::DisplayRole).toString();
  if (bChecked)
  {
    AddKinks({sKink});
  }
  else
  {
    RemoveKinks({sKink});
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotProjectRenamed(qint32 iId)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QReadLocker locker(&m_spCurrentProject->m_rwLock);
  qint32 iThisId = m_spCurrentProject->m_iId;
  QString sName = m_spCurrentProject->m_sName;
  locker.unlock();

  if (iId == iThisId)
  {
    m_spUi->pTitleLineEdit->blockSignals(true);
    m_spUi->pTitleLineEdit->setText(sName);
    m_spUi->pTitleLineEdit->blockSignals(false);
    emit SignalProjectEdited();
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::SlotRemoveKinkClicked()
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  QString sKink = sender()->property(c_sKinkRootWidgetProperty).toString();
  RemoveKinks({sKink});
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::AddKinks(QStringList vsKinks)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr!= spDbManager)
  {
    std::vector<tspKink> vspKinks;
    for (const QString& sKing : qAsConst(vsKinks))
    {
      AddKinks({spDbManager->FindKink(sKing)});
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::AddKinks(std::vector<tspKink> vspKinks)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  CFlowLayout* pLayout = dynamic_cast<CFlowLayout*>(m_spUi->pFetishListWidget->layout());
  if (nullptr != pLayout)
  {
    for (const auto& spKink : vspKinks)
    {
      if (nullptr == spKink) { continue; }

      if (m_vspKinks.end() != std::find_if(m_vspKinks.begin(), m_vspKinks.end(),
        [&spKink](const tspKink& left) -> bool {
        QReadLocker lockerLeft(&left->m_rwLock);
        QReadLocker lockerRight(&spKink->m_rwLock);
        return left->m_sName == spKink->m_sName && left->m_sType == spKink->m_sType;
      })) continue;

      m_vspKinks.push_back(spKink);
      m_spUi->pFetishLineEdit->setText("");

      // Farbe für Kategorie ausrechnen
      QCryptographicHash hasher(QCryptographicHash::Md4);
      hasher.addData(spKink->m_sType.toUtf8());
      QByteArray hashedArr = hasher.result();
      QDataStream ds(hashedArr);
      unsigned short r = 0;
      unsigned short g = 0;
      unsigned short b = 0;
      ds >> r >> g >> b;
      QColor hashColor(r & 0xFF, g & 0xFF, b & 0xFF);

      // calculate foreground / text color
      double dLuminance = (0.299 * hashColor.red() +
                           0.587 * hashColor.green() +
                           0.114 * hashColor.blue()) / 255;
      QColor foregroundColor = Qt::white;
      if (dLuminance > 0.5)
      {
        foregroundColor = Qt::black;
      }

      QReadLocker locker(&spKink->m_rwLock);
      QWidget* pRoot = new QWidget(m_spUi->pFetishListWidget);
      QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
      sizePolicy.setHorizontalStretch(0);
      sizePolicy.setVerticalStretch(0);
      sizePolicy.setHeightForWidth(pRoot->sizePolicy().hasHeightForWidth());
      pRoot->setSizePolicy(sizePolicy);
      pRoot->setObjectName(spKink->m_sName);
      pRoot->setStyleSheet(QString("QWidget { background-color: %1; border-radius: 5px; }")
                           .arg(hashColor.name()));
      pRoot->setToolTip(!spKink->m_sDescribtion.isEmpty() ?
                         spKink->m_sDescribtion : tr("No Describtion available."));

      QHBoxLayout* pRootLayout = new QHBoxLayout(pRoot);
      pRoot->setLayout(pRootLayout);

      QLabel* pLabel = new QLabel(spKink->m_sName, pRoot);
      pLabel->setStyleSheet(QString("QLabel { background-color: transparent; color: %1; }")
                            .arg(foregroundColor.name()));
      pRootLayout->addWidget(pLabel);

      QPushButton* pRemove = new QPushButton(pRoot);
      //pRemove->setObjectName("CloseButton");
      pRemove->setText("X");
      pRemove->setFlat(true);
      QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
      sizePolicy3.setHorizontalStretch(0);
      sizePolicy3.setVerticalStretch(0);
      sizePolicy3.setHeightForWidth(pRemove->sizePolicy().hasHeightForWidth());
      pRemove->setSizePolicy(sizePolicy3);
      pRemove->setMinimumSize(QSize(24, 24));
      pRemove->setMaximumSize(QSize(24, 24));
      pRemove->setStyleSheet(QString("QPushButton { background-color: transparent; color: %1;"
                                     "border: none; }")
                             .arg(foregroundColor.name()));
      pRemove->setProperty(c_sKinkRootWidgetProperty, spKink->m_sName);
      connect(pRemove, &QPushButton::clicked,
              this, &::CEditorProjectSettingsWidget::SlotRemoveKinkClicked);
      pRootLayout->addWidget(pRemove);

      pLayout->addWidget(pRoot);

      KinkModel()->SetSelections(QStringList() << spKink->m_sName);
    }
  }
  else
  {
    qWarning() << tr("pFetishListWidget has no FlowLayout.");
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::RemoveKinks(QStringList vsKinks)
{
  WIDGET_INITIALIZED_GUARD
  if (nullptr == m_spCurrentProject) { return; }

  auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
  if (nullptr!= spDbManager)
  {
    std::vector<tspKink> vspKinks;
    for (const QString& sKing : qAsConst(vsKinks))
    {
      CFlowLayout* pLayout = dynamic_cast<CFlowLayout*>(m_spUi->pFetishListWidget->layout());
      if (nullptr != pLayout)
      {
        for (qint32 i = 0; pLayout->count() > i; ++i)
        {
          if (nullptr != pLayout->itemAt(i)->widget() &&
              pLayout->itemAt(i)->widget()->objectName() == sKing)
          {
            QLayoutItem* pItem = pLayout->takeAt(i);
            delete pItem->widget();
            delete pItem;
            break;
          }
        }

        for (auto it = m_vspKinks.begin(); m_vspKinks.end() != it; ++it)
        {
          QReadLocker locker(&(*it)->m_rwLock);
          if ((*it)->m_sName == sKing)
          {
            m_vspKinks.erase(it);
            break;
          }
        }

        CFlowLayout* pLayout = dynamic_cast<CFlowLayout*>(m_spUi->pFetishListWidget->layout());
        if (nullptr != pLayout)
        {
          pLayout->update();
        }
        KinkModel()->ResetSelections(QStringList() << sKing);
        emit SignalProjectEdited();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::OnActionBarAboutToChange()
{
  if (nullptr != ActionBar())
  {
    // Nothing to do
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorProjectSettingsWidget::OnActionBarChanged()
{
  if (nullptr != ActionBar())
  {
    ActionBar()->ShowProjectSettingsActionBar();
  }
}
