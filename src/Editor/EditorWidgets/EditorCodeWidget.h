#ifndef EDITORCODEWIDGET_H
#define EDITORCODEWIDGET_H

#include "EditorDebuggableWidget.h"
#include "ui_EditorCodeWidget.h"
#include "ui_EditorActionBar.h"
#include <QPointer>
#include <memory>

class CBackgroundSnippetOverlay;
class CCodeWidgetTutorialStateSwitchHandler;
class CDatabaseManager;
class CEditorHighlighter;
class CFilteredEditorEditableFileModel;
class CSettings;
class QStandardItemModel;
namespace Ui {
  class CEditorCodeWidget;
}
struct SResource;
struct SScene;
typedef std::shared_ptr<SResource> tspResource;
typedef std::shared_ptr<SScene> tspScene;

//----------------------------------------------------------------------------------------
//
class CEditorCodeWidget : public CEditorDebuggableWidget
{
  Q_OBJECT

public:
  explicit CEditorCodeWidget(QWidget* pParent = nullptr);
  ~CEditorCodeWidget() override;

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spProject) override;
  void UnloadProjectImpl() override;
  void SaveProject() override;
  void OnHidden() override;
  void OnShown() override;

  void LoadResource(tspResource spResource);

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

protected slots:
  void on_pResourceComboBox_currentIndexChanged(qint32 iIndex);
  void SlotCodeEditContentsChange(qint32 iPos, qint32 iDel, qint32 iAdd);
  void SlotDebugStarted();
  void SlotFileChangedExternally(const QString& sName);
  void SlotShowOverlay();
  void SlotRowsInserted(const QModelIndex& parent, int iFirst, int iLast);
  void SlotRowsRemoved(const QModelIndex& parent, int iFirst, int iLast);

private:
  QString CachedResourceName(qint32 iIndex);
  tSceneToDebug GetScene();
  void ReloadEditor(qint32 iIndex);

  std::shared_ptr<Ui::CEditorCodeWidget>                 m_spUi;
  std::shared_ptr<CCodeWidgetTutorialStateSwitchHandler> m_spTutorialStateSwitchHandler;
  std::shared_ptr<CSettings>                             m_spSettings;
  std::weak_ptr<CDatabaseManager>                        m_wpDbManager;
  QPointer<CFilteredEditorEditableFileModel>             m_pFilteredScriptModel;
  QPointer<QStandardItemModel>                           m_pDummyModel;

  bool                                                   m_bChangingIndex;
  QString                                                m_sLastCachedScript;
};

#endif // EDITORCODEWIDGET_H
