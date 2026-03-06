#include "SubflowNodeModelWidget.h"
#include "ui_SubflowNodeModelWidget.h"

#include "Utils/UndoRedoFilter.h"

CSubflowNodeModelWidget::CSubflowNodeModelWidget(QWidget *parent) :
    QWidget(parent),
    m_spUi(std::make_unique<Ui::CSubflowNodeModelWidget>())
{
  m_spUi->setupUi(this);

  setPalette(Qt::transparent);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_NoSystemBackground);
  setAutoFillBackground(false);

  m_spUi->CreateNewFlow->setProperty("styleSmall", true);

  new CUndoRedoFilter(m_spUi->pNameLineEdit, nullptr);
}

CSubflowNodeModelWidget::~CSubflowNodeModelWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWidget::SetFlow(const QString& sFlow)
{
  QSignalBlocker blocker(m_spUi->pFlowComboBox);
  qint32 iIdx = m_spUi->pFlowComboBox->findData(sFlow);
  m_spUi->pFlowComboBox->setCurrentIndex(iIdx);
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWidget::SetProject(const tspProject& spProject)
{
  QSignalBlocker blocker(m_spUi->pFlowComboBox);
  m_spUi->pFlowComboBox->clear();

  m_spUi->pFlowComboBox->addItem("<No Flow>", "");

  for (const auto& [sName, tspResource] : spProject->m_baseData.m_spResourcesMap)
  {
    QReadLocker locker(&tspResource->m_rwLock);
    switch (tspResource->m_type)
    {
      case EResourceType::eFlow:
        m_spUi->pFlowComboBox->addItem(sName, sName);
        break;
      default: break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWidget::SetNodeName(const QString& sFlow)
{
  QSignalBlocker blocker(m_spUi->pNameLineEdit);
  m_spUi->pNameLineEdit->setText(sFlow);
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWidget::OnFlowAdded(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pFlowComboBox);
  m_spUi->pFlowComboBox->addItem(sName, sName);
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWidget::OnFlowRenamed(const QString& sOldName, const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pFlowComboBox);
  qint32 iIdx = m_spUi->pFlowComboBox->findData(sOldName);
  bool bWasSelected = iIdx == m_spUi->pFlowComboBox->currentIndex();
  if (-1 != iIdx)
  {
    m_spUi->pFlowComboBox->removeItem(iIdx);
  }
  m_spUi->pFlowComboBox->addItem(sName, sName);
  if (bWasSelected)
  {
    m_spUi->pFlowComboBox->setCurrentIndex(m_spUi->pFlowComboBox->findData(sName));
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWidget::OnFlowRemoved(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pFlowComboBox);
  qint32 iIdx = m_spUi->pFlowComboBox->findData(sName);
  if (-1 != iIdx)
  {
    m_spUi->pFlowComboBox->removeItem(iIdx);
  }
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWidget::on_pNameLineEdit_editingFinished()
{
  emit SignalNameChanged(m_spUi->pNameLineEdit->text());
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWidget::on_pFlowComboBox_currentIndexChanged(qint32)
{
  emit SignalFlowChanged(m_spUi->pFlowComboBox->currentData().toString());
}

//----------------------------------------------------------------------------------------
//
void CSubflowNodeModelWidget::on_CreateNewFlow_clicked()
{
  emit SignalAddNodeFileClicked();
}
