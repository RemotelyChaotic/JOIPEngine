#include "SceneNodeModelWidget.h"
#include "ui_SceneNodeModelWidget.h"

CSceneNodeModelWidget::CSceneNodeModelWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(std::make_unique<Ui::CSceneNodeModelWidget>())
{
  m_spUi->setupUi(this);
  m_spUi->pRandomRadio->setChecked(true);

  setPalette(Qt::transparent);
  setAttribute( Qt::WA_TranslucentBackground, true );
  setAttribute( Qt::WA_OpaquePaintEvent, true );
  setAutoFillBackground(false);
  setAttribute(Qt::WA_NoSystemBackground);
}

CSceneNodeModelWidget::~CSceneNodeModelWidget()
{
}
