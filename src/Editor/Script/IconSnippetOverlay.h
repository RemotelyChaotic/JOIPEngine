#ifndef ICONSNIPPETOVERLAY_H
#define ICONSNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"
#include <QPointer>

class CScriptEditorWidget;
class CResourceTreeItemModel;
namespace Ui {
  class CIconSnippetOverlay;
}

//----------------------------------------------------------------------------------------
//
class CIconSnippetOverlay : public CCodeSnippetOverlayBase
{
  Q_OBJECT

public:
  explicit CIconSnippetOverlay(QWidget* pParent = nullptr);
  ~CIconSnippetOverlay() override;

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

public slots:
  void Resize() override;

protected slots:
  void on_pResourceLineEdit_editingFinished();
  void on_pShowCheckBox_toggled(bool bState);
  void on_pHideCheckBox_toggled(bool bState);
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_CloseButton_clicked();
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  std::unique_ptr<Ui::CIconSnippetOverlay> m_spUi;
  SIconSnippetData                         m_data;
  QSize                                    m_preferredSize;
};

#endif // ICONSNIPPETOVERLAY_H
