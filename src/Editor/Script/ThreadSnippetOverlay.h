#ifndef THREADSNIPPETOVERLAY_H
#define THREADSNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"
#include <QPointer>
#include <QWidget>

class CScriptEditorWidget;
namespace Ui {
  class CThreadSnippetOverlay;
}

class CThreadSnippetOverlay : public CCodeSnippetOverlayBase
{
  Q_OBJECT

public:
  explicit CThreadSnippetOverlay(QWidget* pParent = nullptr);
  ~CThreadSnippetOverlay() override;

public slots:
  void Resize() override;

protected slots:
  void on_pSleepSpinBox_valueChanged(double dValue);
  void on_pSkippableCheckBox_toggled(bool bStatus);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CThreadSnippetOverlay> m_spUi;
  SThreadSnippetOverlay                      m_data;
  QSize                                      m_preferredSize;
};

#endif // THREADSNIPPETOVERLAY_H
