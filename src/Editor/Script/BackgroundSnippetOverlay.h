#ifndef BACKGROUNDSNIPPETOVERLAY_H
#define BACKGROUNDSNIPPETOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QColor>
#include <QPointer>
#include <memory>

class CScriptEditorWidget;
class CResourceTreeItemModel;
namespace Ui {
  class CBackgroundSnippetOverlay;
}

struct SBackgroundSnippetData
{
  bool      m_bUseResource = false;
  QString   m_sCurrentResource = QString();
  bool      m_bUseColor = false;
  QColor    m_color = QColor();
};

//----------------------------------------------------------------------------------------
//
class CBackgroundSnippetOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CBackgroundSnippetOverlay(CScriptEditorWidget* pParent = nullptr);
  ~CBackgroundSnippetOverlay() override;

  void Initialize(CResourceTreeItemModel* pResourceTreeModel);

signals:
  void SignalBackgroundCode(const QString& code);

public slots:
  void Climb() override;
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
  QPointer<CScriptEditorWidget>                  m_pEditor;
  bool                                           m_bInitialized;
  SBackgroundSnippetData                         m_data;
  QSize                                          m_preferredSize;
};

#endif // BACKGROUNDSNIPPETOVERLAY_H
