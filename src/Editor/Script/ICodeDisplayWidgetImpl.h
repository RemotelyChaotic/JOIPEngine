#ifndef ICODEDISPLAYWIDGETIMPL_H
#define ICODEDISPLAYWIDGETIMPL_H

#include <QPointer>

namespace Ui {
  class CEditorActionBar;
}

//----------------------------------------------------------------------------------------
//
class ICodeDisplayWidgetImpl
{
public:
  ICodeDisplayWidgetImpl() {};
  virtual ~ICodeDisplayWidgetImpl() = default;

  virtual void Clear() = 0;
  virtual void ExecutionError(QString sException, qint32 iLine, QString sStack) = 0;
  virtual void InsertGeneratedCode(const QString& sCode) = 0;
  virtual void ResetWidget() = 0;
  virtual void SetContent(const QString& sContent) = 0;
  virtual void SetHighlightDefinition(const QString& sType) = 0;
  virtual void ShowButtons(Ui::CEditorActionBar* pActionBar) = 0;
  virtual void Update() = 0;

  virtual QString GetCurrentText() const = 0;
};

#endif // ICODEDISPLAYWIDGETIMPL_H
