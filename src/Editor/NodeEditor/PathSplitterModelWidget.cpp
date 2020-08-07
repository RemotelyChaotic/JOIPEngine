#include "PathSplitterModelWidget.h"
#include "Systems/Scene.h"
#include "ui_PathSplitterModelWidget.h"

CPathSplitterModelWidget::CPathSplitterModelWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CPathSplitterModelWidget>())
{
  m_spUi->setupUi(this);
  m_spUi->pRandomRadio->setChecked(true);

  setPalette(Qt::transparent);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_NoSystemBackground);
  setAutoFillBackground(false);
}

CPathSplitterModelWidget::~CPathSplitterModelWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::SetTransitionType(qint32 iType)
{
  switch(iType)
  {
    case ESceneTransitionType::eRandom:
      m_spUi->pRandomRadio->setChecked(true);
      break;
    case ESceneTransitionType::eSelection:
      m_spUi->pSelectionRadio->setChecked(true);
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
void CPathSplitterModelWidget::on_pRandomRadio_clicked(bool bChecked)
{
  if (bChecked)
  {
    emit SignalTransitionTypeChanged(ESceneTransitionType::eRandom);
  }
}

//----------------------------------------------------------------------------------------
//
void CPathSplitterModelWidget::on_pSelectionRadio_clicked(bool bChecked)
{
  if (bChecked)
  {
    emit SignalTransitionTypeChanged(ESceneTransitionType::eSelection);
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
