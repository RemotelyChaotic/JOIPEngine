#include "SceneNodeModelWidget.h"
#include "ui_SceneNodeModelWidget.h"

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

  m_spUi->pSceneNameLineEdit->setStyleSheet("{ background: rgb(0, 255, 255); selection-background-color: rgb(233, 99, 0); }");
}

CSceneNodeModelWidget::~CSceneNodeModelWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::SetName(const QString& sName)
{
  m_spUi->pSceneNameLineEdit->blockSignals(true);
  m_spUi->pSceneNameLineEdit->setText(sName);
  m_spUi->pSceneNameLineEdit->blockSignals(false);
}

//----------------------------------------------------------------------------------------
//
void CSceneNodeModelWidget::on_pSceneNameLineEdit_editingFinished()
{
  emit SignalNameChanged(m_spUi->pSceneNameLineEdit->text());
}