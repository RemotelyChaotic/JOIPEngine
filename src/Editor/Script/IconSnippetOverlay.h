#ifndef ICONSNIPPETOVERLAY_H
#define ICONSNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <memory>

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
  explicit CIconSnippetOverlay(QWidget* pParent = nullptr);
  ~CIconSnippetOverlay() override;

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

signals:
  void SignalIconCode(const QString& code);

public slots:
  void Resize() override;

protected slots:
  void on_pResourceLineEdit_editingFinished();
  void on_pShowCheckBox_toggled(bool bState);
  void on_pHideCheckBox_toggled(bool bState);
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  std::unique_ptr<Ui::CIconSnippetOverlay> m_spUi;
  bool                                     m_bInitialized;
  SIconSnippetData                         m_data;
};

#endif // ICONSNIPPETOVERLAY_H
