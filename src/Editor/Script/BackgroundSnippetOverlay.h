#ifndef BACKGROUNDSNIPPETOVERLAY_H
#define BACKGROUNDSNIPPETOVERLAY_H

#include "CodeSnippetOverlayBase.h"
#include <QColor>
#include <QPointer>

class CScriptEditorWidget;
class CResourceTreeItemModel;
namespace Ui {
  class CBackgroundSnippetOverlay;
}

//----------------------------------------------------------------------------------------
//
class CBackgroundSnippetOverlay : public CCodeSnippetOverlayBase
{
  Q_OBJECT

public:
  explicit CBackgroundSnippetOverlay(QWidget* pParent = nullptr);
  ~CBackgroundSnippetOverlay() override;

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

public slots:
  void Resize() override;

protected slots:
  void on_pResourceCheckBox_toggled(bool bState);
  void on_pResourceLineEdit_editingFinished();
  void on_pColorCheckBox_toggled(bool bState);
  void on_pColorWidget_SignalColorChanged(const QColor& color);
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  std::unique_ptr<Ui::CBackgroundSnippetOverlay> m_spUi;
  SBackgroundSnippetData                         m_data;
  QSize                                          m_preferredSize;
};

#endif // BACKGROUNDSNIPPETOVERLAY_H
