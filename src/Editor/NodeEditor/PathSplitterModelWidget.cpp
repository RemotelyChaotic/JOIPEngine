#include "PathSplitterModelWidget.h"
#include "ui_PathSplitterModelWidget.h"
#include "Systems/Scene.h"
#include "Utils/UndoRedoFilter.h"

#include <QButtonGroup>

namespace
{
  constexpr static char c_sDefaultLayout[] = R"(import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import JoipEngine 1.3
import JOIP.core 1.3
import JOIP.db 1.1

PlayerTansition {
    // TODO: use function returnValue(index) to return the player choice
    anchors.fill: parent

    property int spacing: 5

    readonly property bool isMobile: Settings.platform === "Android"
    readonly property int dominantHand: Settings.dominantHand
    readonly property int iconWidth: isMobile ? 32 : 64
    readonly property int iconHeight: isMobile ? 32 : 64
    property bool isLandscape: { return width > height; }
}
)";
}

//----------------------------------------------------------------------------------------
//
CPathSplitterModelWidget::CPathSplitterModelWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CPathSplitterModelWidget>())
{
  m_spUi->setupUi(this);
  m_spUi->pRandomRadio->setChecked(true);
  m_spUi->pButtonLabelsGroupBox->setChecked(false);

  setPalette(Qt::transparent);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_NoSystemBackground);
  setAutoFillBackground(false);

  m_spUi->AddLayoutFile->setProperty("styleSmall", true);

  new CUndoRedoFilter(m_spUi->pLabel1, nullptr);
  new CUndoRedoFilter(m_spUi->pLabel2, nullptr);
  new CUndoRedoFilter(m_spUi->pLabel3, nullptr);
  new CUndoRedoFilter(m_spUi->pLabel4, nullptr);
}

CPathSplitterModelWidget::~CPathSplitterModelWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::SetCustomLayout(bool bEnabled, const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pLayoutComboBox);
  QSignalBlocker blocker2(m_spUi->pCustomTransitionCheckBox);
  qint32 iIdx = m_spUi->pLayoutComboBox->findData(sName);
  m_spUi->pLayoutComboBox->setCurrentIndex(iIdx);
  m_spUi->pCustomTransitionCheckBox->setChecked(bEnabled);
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::SetProject(const tspProject& spProject)
{
  QSignalBlocker blocker21(m_spUi->pLayoutComboBox);
  m_spUi->pLayoutComboBox->clear();

  m_spUi->pLayoutComboBox->addItem("<Transition Layout>", "");

  for (const auto& [sName, tspResource] : spProject->m_spResourcesMap)
  {
    QReadLocker locker(&tspResource->m_rwLock);
    switch (tspResource->m_type)
    {
      case EResourceType::eLayout:
        m_spUi->pLayoutComboBox->addItem(sName, sName);
        break;
      default: break;
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::SetTransitionType(qint32 iType)
{
  switch(iType)
  {
    case ESceneTransitionType::eRandom:
      m_spUi->pRandomRadio->setChecked(true);
      m_spUi->pButtonLabelsGroupBox->setChecked(false);
      break;
    case ESceneTransitionType::eSelection:
      m_spUi->pRandomRadio->setChecked(false);
      m_spUi->pButtonLabelsGroupBox->setChecked(true);
      break;
  default:break;
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::SetTransitionLabel(PortIndex index, const QString& sLabelValue)
{
  switch(index)
  {
    case 0:
      m_spUi->pLabel1->setText(sLabelValue);
      break;
    case 1:
      m_spUi->pLabel2->setText(sLabelValue);
      break;
    case 2:
      m_spUi->pLabel3->setText(sLabelValue);
      break;
    case 3:
      m_spUi->pLabel4->setText(sLabelValue);
      break;
  default:break;
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::OnLayoutAdded(const QString& sName)
{
  QSignalBlocker blocker(m_spUi->pLayoutComboBox);
  m_spUi->pLayoutComboBox->addItem(sName, sName);
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::OnLayoutRenamed(const QString& sOldName, const QString& sName)
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
void CPathSplitterModelWidget::OnLayoutRemoved(const QString& sName)
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
void CPathSplitterModelWidget::on_pRandomRadio_clicked(bool bChecked)
{
  if (bChecked)
  {
    {
      QSignalBlocker b(m_spUi->pButtonLabelsGroupBox);
      m_spUi->pButtonLabelsGroupBox->setChecked(false);
    }
    emit SignalTransitionTypeChanged(ESceneTransitionType::eRandom);
  }
  else
  {
    {
      QSignalBlocker b(m_spUi->pButtonLabelsGroupBox);
      m_spUi->pButtonLabelsGroupBox->setChecked(true);
    }
    emit SignalTransitionTypeChanged(ESceneTransitionType::eSelection);
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::on_pButtonLabelsGroupBox_clicked(bool bChecked)
{
  if (bChecked)
  {
    {
      QSignalBlocker b(m_spUi->pRandomRadio);
      m_spUi->pRandomRadio->setChecked(false);
    }
    emit SignalTransitionTypeChanged(ESceneTransitionType::eSelection);
  }
  else
  {
    {
      QSignalBlocker b(m_spUi->pRandomRadio);
      m_spUi->pRandomRadio->setChecked(true);
    }
    emit SignalTransitionTypeChanged(ESceneTransitionType::eRandom);
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::on_pLabel1_editingFinished()
{
  emit SignalTransitionLabelChanged(0, m_spUi->pLabel1->text());
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::on_pLabel2_editingFinished()
{
  emit SignalTransitionLabelChanged(1, m_spUi->pLabel2->text());
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::on_pLabel3_editingFinished()
{
  emit SignalTransitionLabelChanged(2, m_spUi->pLabel3->text());
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::on_pLabel4_editingFinished()
{
  emit SignalTransitionLabelChanged(3, m_spUi->pLabel4->text());
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::on_pCustomTransitionCheckBox_clicked(bool bChecked)
{
  emit SignalCustomTransitionChanged(bChecked, bChecked ?
                                       m_spUi->pLayoutComboBox->currentData().toString() :
                                       QString());
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::on_pLayoutComboBox_currentIndexChanged(qint32)
{
  bool bChecked = m_spUi->pCustomTransitionCheckBox->isChecked();
  emit SignalCustomTransitionChanged(bChecked, bChecked ?
                                       m_spUi->pLayoutComboBox->currentData().toString() :
                                       QString());
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::on_AddLayoutFile_clicked()
{
  emit SignalAddLayoutFileClicked(QString(c_sDefaultLayout));
}
