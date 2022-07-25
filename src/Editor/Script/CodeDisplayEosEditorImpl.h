#ifndef CCODEDISPLAYEOSEDITORIMPL_H
#define CCODEDISPLAYEOSEDITORIMPL_H

#include "ICodeDisplayWidgetImpl.h"

#include <QTreeView>

class CCodeDisplayEosEditorImpl : public ICodeDisplayWidgetImpl
{
public:
  CCodeDisplayEosEditorImpl(QPointer<QTreeView> pTarget);
  ~CCodeDisplayEosEditorImpl() override;

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
  QPointer<QTreeView> m_pTarget;
};

#endif // CCODEDISPLAYEOSEDITORIMPL_H
