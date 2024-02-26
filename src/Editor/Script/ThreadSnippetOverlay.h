#ifndef THREADSNIPPETOVERLAY_H
#define THREADSNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"
#include <QPointer>
#include <QWidget>

class CDatabaseManager;
class CResourceTreeItemModel;
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

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

public slots:
  void Resize() override;

protected slots:
  void on_pSleepCheckBox_toggled(bool bStatus);
  void on_pSleepSpinBox_valueChanged(double dValue);
  void on_pSkippableCheckBox_toggled(bool bStatus);
  void on_pKillThreadTextBox_toggled(bool bStatus);
  void on_pKillThreadName_editingFinished();
  void on_pRunAsynchCheckBox_toggled(bool bStatus);
  void on_pIdLineEdit_editingFinished();
  void on_pRunAsynchLineEdit_editingFinished();
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  std::unique_ptr<Ui::CThreadSnippetOverlay> m_spUi;
  std::weak_ptr<CDatabaseManager>            m_wpDbManager;
  SThreadSnippetOverlay                      m_data;
  QSize                                      m_preferredSize;
};

#endif // THREADSNIPPETOVERLAY_H
