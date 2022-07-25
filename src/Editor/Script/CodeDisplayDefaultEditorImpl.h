#ifndef CCODEDISPLAYDEFAULTEDITORIMPL_H
#define CCODEDISPLAYDEFAULTEDITORIMPL_H

#include "ICodeDisplayWidgetImpl.h"

class CScriptEditorWidget;

class CCodeDisplayDefaultEditorImpl : public ICodeDisplayWidgetImpl
{
public:
  CCodeDisplayDefaultEditorImpl(QPointer<CScriptEditorWidget> pTarget);
  ~CCodeDisplayDefaultEditorImpl() override;

  void Clear() override;
  void ExecutionError(QString sException, qint32 iLine, QString sStack) override;
  void InsertGeneratedCode(const QString& sCode) override;
  void ResetWidget() override;
  void SetContent(const QString& sContent) override;
  void SetHighlightDefinition(const QString& sType) override;
  void ShowButtons(Ui::CEditorActionBar* pActionBar) override;
  void Update() override;

  QString GetCurrentText() const override;

private:
  QPointer<CScriptEditorWidget> m_pCodeEdit;
};

#endif // CCODEDISPLAYDEFAULTEDITORIMPL_H
