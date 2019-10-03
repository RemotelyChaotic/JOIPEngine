#ifndef RESOURCESNIPPETOVERLAY_H
#define RESOURCESNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QPointer>
#include <memory>

class CResourceTreeItemModel;
namespace Ui {
  class CResourceSnippetOverlay;
}

class CResourceSnippetOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CResourceSnippetOverlay(QWidget* pParent = nullptr);
  ~CResourceSnippetOverlay() override;

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

signals:
  void SignalResourceCode(const QString& code);

public slots:
  void Resize() override;

protected slots:
  void on_pResourceLineEdit_editingFinished();
  void on_pFilterLineEdit_2_textChanged(const QString& sText);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();
  void SlotCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  std::unique_ptr<Ui::CResourceSnippetOverlay> m_spUi;
  bool                                         m_bInitialized;

  QString                                      m_sResource;
};

#endif // RESOURCESNIPPETOVERLAY_H
