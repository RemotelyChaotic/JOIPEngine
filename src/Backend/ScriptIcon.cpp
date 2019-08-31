#include "ScriptIcon.h"

CScriptIcon::CScriptIcon(QJSEngine* pEngine) :
  QObject(),
  m_pEngine(pEngine)
{

}

CScriptIcon::~CScriptIcon()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptIcon::hide()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptIcon::show(QJSValue resource)
{
  Q_UNUSED(resource);
}
