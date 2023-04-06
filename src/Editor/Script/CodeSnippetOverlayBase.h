#ifndef CCODESNIPPETOVERLAYBASE_H
#define CCODESNIPPETOVERLAYBASE_H

#include "Systems/Script/ICodeGenerator.h"
#include "Widgets/OverlayBase.h"
#include <memory>

typedef std::shared_ptr<struct SProject> tspProject;

class CCodeSnippetOverlayBase : public COverlayBase
{
  Q_OBJECT

public:
  explicit CCodeSnippetOverlayBase(QWidget* pParent = nullptr);
  ~CCodeSnippetOverlayBase() override;

  void SetCurrentScriptType(const QString& sType);

  virtual void LoadProject(tspProject spProject);
  virtual void UnloadProject();

signals:
  void SignalCodeGenerated(const QString& code);

public slots:
  void Climb() override;

protected:
  std::shared_ptr<ICodeGenerator> CodeGenerator() const;

  void SetInitialized(bool bInit);

  tspProject                               m_spCurrentProject;
  bool                                     m_bInitialized;
  QString                                  m_sCurrentScriptType;
};

#endif // CCODESNIPPETOVERLAYBASE_H
