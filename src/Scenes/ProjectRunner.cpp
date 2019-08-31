#include "ProjectRunner.h"
#include "Backend/Project.h"
#include "Backend/Resource.h"
#include "Backend/Scene.h"

#include <QDebug>

CProjectRunner::CProjectRunner(QObject* pParent) :
  QObject (pParent),
  m_spCurrentProject(nullptr)
{
}

CProjectRunner::~CProjectRunner()
{}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::LoadProject(tspProject spProject)
{
  if (nullptr != m_spCurrentProject)
  {
    qWarning() << "Old Project was not unloaded before loading project.";
  }

  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CProjectRunner::UnloadProject()
{
  m_spCurrentProject = nullptr;
}
