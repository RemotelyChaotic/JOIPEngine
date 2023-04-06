#include "CodeSnippetOverlayBase.h"
#include "Systems/Script/CodeGeneratorProvider.h"

CCodeSnippetOverlayBase::CCodeSnippetOverlayBase(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_spCurrentProject(nullptr),
  m_bInitialized(false)
{
}
CCodeSnippetOverlayBase::~CCodeSnippetOverlayBase()
{
}

//----------------------------------------------------------------------------------------
//
void CCodeSnippetOverlayBase::SetCurrentScriptType(const QString& sType)
{
  m_sCurrentScriptType = sType;
}

//----------------------------------------------------------------------------------------
//
void CCodeSnippetOverlayBase::LoadProject(tspProject spProject)
{
  m_spCurrentProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CCodeSnippetOverlayBase::UnloadProject()
{
  m_spCurrentProject = nullptr;
}

//----------------------------------------------------------------------------------------
//
void CCodeSnippetOverlayBase::Climb()
{
  ClimbToFirstInstanceOf("QStackedWidget", false);
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<ICodeGenerator> CCodeSnippetOverlayBase::CodeGenerator() const
{
  return CCodeGeneratorProvider::Instance().Generator(m_sCurrentScriptType);
}

//----------------------------------------------------------------------------------------
//
void CCodeSnippetOverlayBase::SetInitialized(bool bInit)
{
  m_bInitialized = bInit;
}
