#ifndef ICONSNIPPETOVERLAY_H
#define ICONSNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QPointer>
#include <memory>

class CScriptEditorWidget;
class CResourceTreeItemModel;
namespace Ui {
  class CIconSnippetOverlay;
}

struct SIconSnippetData
{
  bool      m_bShow = false;
  QString   m_sCurrentResource = QString();
};

//----------------------------------------------------------------------------------------
//
class CIconSnippetOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CIconSnippetOverlay(CScriptEditorWidget* pParent = nullptr);
  ~CIconSnippetOverlay() override;

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

signals:
  void SignalIconCode(const QString& code);

public slots:
  void Climb() override;
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
  QPointer<CScriptEditorWidget>            m_pEditor;
  bool                                     m_bInitialized;
  SIconSnippetData                         m_data;
  QSize                                    m_preferredSize;
};

#endif // ICONSNIPPETOVERLAY_H
