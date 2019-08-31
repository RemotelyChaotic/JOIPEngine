#include "ScriptTextBox.h"

CScriptTextBox::CScriptTextBox(QJSEngine* pEngine) :
  QObject(),
  m_pEngine(pEngine)
{
}

CScriptTextBox::~CScriptTextBox()
{
}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptTextBox::getBackgroundColor()
{
  return m_pEngine->toScriptValue(QColor());
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setBackgroundColor(QJSValue color)
{
  Q_UNUSED(color);
}

//----------------------------------------------------------------------------------------
//
QJSValue CScriptTextBox::getTextColor()
{
  return m_pEngine->toScriptValue(QColor());
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setTextColor(QJSValue color)
{
  Q_UNUSED(color);
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showButtonPrompts(QJSValue vsLabels)
{
  Q_UNUSED(vsLabels);
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showText(const QString& sText)
{
  Q_UNUSED(sText);
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::clear()
{

}
