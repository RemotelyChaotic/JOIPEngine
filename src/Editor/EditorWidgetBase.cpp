#include "EditorWidgetBase.h"

CEditorWidgetBase::CEditorWidgetBase(QWidget* pParent, CEditorActionBar* pActionBar) :
  QWidget(pParent),
  m_pActionBar(pActionBar),
  m_bInitialized(false)
{

}

CEditorWidgetBase::~CEditorWidgetBase()
{

}
