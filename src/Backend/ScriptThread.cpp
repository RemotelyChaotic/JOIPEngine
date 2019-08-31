#include "ScriptThread.h"


CScriptThread::CScriptThread(QJSEngine* pEngine) :
  QObject(),
  m_pEngine(pEngine)
{

}

CScriptThread::~CScriptThread()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptThread::sleep(qint32 iTimeMs)
{
  Q_UNUSED(iTimeMs);
}
