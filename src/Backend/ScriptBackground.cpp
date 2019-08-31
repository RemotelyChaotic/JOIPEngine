#include "ScriptBackground.h"

CScriptBackground::CScriptBackground(QJSEngine* pEngine) :
  QObject(),
  m_pEngine(pEngine)
{
}

CScriptBackground::~CScriptBackground()
{
}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptBackground::getBackgroundColor()
{
  return m_pEngine->toScriptValue(QColor());
}

//----------------------------------------------------------------------------------------
//
void CScriptBackground::setBackgroundColor(QJSValue color)
{
  Q_UNUSED(color);
}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptBackground::getBackgroundTexture()
{
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
void CScriptBackground::setBackgroundTexture(QJSValue resource)
{
  Q_UNUSED(resource);
}
