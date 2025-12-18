#include "SceneNodeModelWidget.h"
#include "ui_SceneNodeModelWidget.h"

#include "Utils/UndoRedoFilter.h"

CSceneNodeModelWidget::CSceneNodeModelWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CSceneNodeModelWidget>())
{
  m_spUi->setupUi(this);

  setPalette(Qt::transparent);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_NoSystemBackground);
  setAutoFillBackground(false);

  {
    QSignalBlocker blocker3(m_spUi->pSceneModeComboBox);
    m_spUi->pSceneModeComboBox->clear();
    m_spUi->pSceneModeComboBox->addItem("Linear", ESceneMode::eLinear);
    m_spUi->pSceneModeComboBox->addItem("Event Driven", ESceneMode::eEventDriven);
  }

  m_spUi->AddLayoutFile->setProperty("styleSmall", true);
  m_spUi->AddScriptFile->setProperty("styleSmall", true);

  new CUndoRedoFilter(m_spUi->pSceneNameLineEdit, nullptr);
}

CSceneNodeModelWidget::~CSceneNodeModelWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::SetCanStartHere(bool bValue)
{
  QSignalBlocker blocker(m_spUi->pCanStartHere);
  m_spUi->pCanStartHere->setChecked(bValue);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::SetName(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pSceneNameLineEdit);
  m_spUi->pSceneNameLineEdit->setText(sName);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::SetProject(const tspProject& spProject)
{
  QSignalBlocker blocker1(m_spUi->pScriptComboBox);
  m_spUi->pScriptComboBox->clear();
  QSignalBlocker blocker21(m_spUi->pLayoutComboBox);
  m_spUi->pLayoutComboBox->clear();

  m_spUi->pScriptComboBox->addItem("<No Script>", "");
  m_spUi->pLayoutComboBox->addItem("<Use Project Layout>", "");

  for (const auto& [sName, tspResource] : spProject->m_baseData.m_spResourcesMap)
  {
    QReadLocker locker(&tspResource->m_rwLock);
    switch (tspResource->m_type)
    {
      case EResourceType::eScript:
        m_spUi->pScriptComboBox->addItem(sName, sName);
        break;
      case EResourceType::eLayout:
        m_spUi->pLayoutComboBox->addItem(sName, sName);
        break;
      default: break;
    }
  }

  m_spUi->FileIcon->SetCurrentProject(spProject);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::SetScript(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pScriptComboBox);
  qint32 iIdx = m_spUi->pScriptComboBox->findData(sName);
  m_spUi->pScriptComboBox->setCurrentIndex(iIdx);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::SetLayout(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pLayoutComboBox);
  qint32 iIdx = m_spUi->pLayoutComboBox->findData(sName);
  m_spUi->pLayoutComboBox->setCurrentIndex(iIdx);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::SetResourceItemModel(QAbstractItemModel* pModel)
{
  m_spUi->FileIcon->SetResourceModel(pModel);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::SetSceneMode(ESceneMode mode)
{
  QSignalBlocker blocker(m_spUi->pSceneModeComboBox);
  qint32 iIdx = m_spUi->pSceneModeComboBox->findData(mode._to_integral());
  m_spUi->pSceneModeComboBox->setCurrentIndex(iIdx);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::SetTileResource(const QString& sName)
{
  m_spUi->FileIcon->SetCurrentResource(sName);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::OnLayoutAdded(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pLayoutComboBox);
  m_spUi->pLayoutComboBox->addItem(sName, sName);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::OnLayoutRenamed(const QString& sOldName, const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pLayoutComboBox);
  qint32 iIdx = m_spUi->pLayoutComboBox->findData(sOldName);
  bool bWasSelected = iIdx == m_spUi->pLayoutComboBox->currentIndex();
  if (-1 != iIdx)
  {
    m_spUi->pLayoutComboBox->removeItem(iIdx);
  }
  m_spUi->pLayoutComboBox->addItem(sName, sName);
  if (bWasSelected)
  {
    m_spUi->pLayoutComboBox->setCurrentIndex(m_spUi->pLayoutComboBox->findData(sName));
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::OnLayoutRemoved(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pLayoutComboBox);
  qint32 iIdx = m_spUi->pLayoutComboBox->findData(sName);
  if (-1 != iIdx)
  {
    m_spUi->pLayoutComboBox->removeItem(iIdx);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::OnScriptAdded(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pScriptComboBox);
  m_spUi->pScriptComboBox->addItem(sName, sName);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::OnScriptRenamed(const QString& sOldName, const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pScriptComboBox);
  qint32 iIdx = m_spUi->pScriptComboBox->findData(sOldName);
  bool bWasSelected = iIdx == m_spUi->pScriptComboBox->currentIndex();
  if (-1 != iIdx)
  {
    m_spUi->pScriptComboBox->removeItem(iIdx);
  }
  m_spUi->pScriptComboBox->addItem(sName, sName);
  if (bWasSelected)
  {
    m_spUi->pScriptComboBox->setCurrentIndex(m_spUi->pScriptComboBox->findData(sName));
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::OnScriptRemoved(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pScriptComboBox);
  qint32 iIdx = m_spUi->pScriptComboBox->findData(sName);
  if (-1 != iIdx)
  {
    m_spUi->pScriptComboBox->removeItem(iIdx);
  }
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::on_pSceneModeComboBox_currentIndexChanged(qint32)
{
  emit SignalSceneModeChanged(m_spUi->pSceneModeComboBox->currentData().toInt());
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::on_FileIcon_SignalResourcePicked(const QString& sOld,
                                                             const QString& sNew)
{
  emit SignalTitleResourceChanged(sOld, sNew);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::on_pCanStartHere_toggled(bool bCanStartHere)
{
  emit SignalCanStartHereChanged(bCanStartHere);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::on_AddScriptFile_clicked()
{
  emit SignalAddScriptFileClicked(QString());
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::on_AddLayoutFile_clicked()
{
  emit SignalAddLayoutFileClicked(QString());
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::on_pSceneNameLineEdit_editingFinished()
{
  emit SignalNameChanged(m_spUi->pSceneNameLineEdit->text());
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::on_pScriptComboBox_currentIndexChanged(qint32)
{
  emit SignalScriptChanged(m_spUi->pScriptComboBox->currentData().toString());
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::on_pLayoutComboBox_currentIndexChanged(qint32)
{
  emit SignalLayoutChanged(m_spUi->pLayoutComboBox->currentData().toString());
}
