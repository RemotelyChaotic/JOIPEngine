#ifndef EDITORCODEWIDGET_H
#define EDITORCODEWIDGET_H

#include "EditorWidgetBase.h"
#include "ui_EditorCodeWidget.h"
#include "ui_EditorActionBar.h"
#include <QPointer>
#include <memory>

class CBackgroundSnippetOverlay;
class CCodeWidgetTutorialStateSwitchHandler;
class CDatabaseManager;
class CEditorHighlighter;
class CFilteredEditorEditableFileModel;
class CScriptRunner;
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
class CEditorCodeWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorCodeWidget(QWidget* pParent = nullptr);
  ~CEditorCodeWidget() override;

  void EditedProject() override {}
  void Initialize() override;
  void LoadProject(tspProject spProject) override;
  void UnloadProject() override;
  void SaveProject() override;
  void OnHidden() override;
  void OnShown() override;

  void LoadResource(tspResource spResource);

signals:
  void SignalDebugFinished();

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

protected slots:
  void on_pResourceComboBox_currentIndexChanged(qint32 iIndex);
  void SlotCodeEditContentsChange(qint32 iPos, qint32 iDel, qint32 iAdd);
  void SlotDebugStart();
  void SlotDebugStop();
  void SlotDebugUnloadFinished();
  void SlotFileChangedExternally(const QString& sName);
  void SlotShowOverlay();
  void SlotRowsInserted(const QModelIndex& parent, int iFirst, int iLast);
  void SlotRowsRemoved(const QModelIndex& parent, int iFirst, int iLast);

private:
  QString CachedResourceName(qint32 iIndex);
  void ReloadEditor(qint32 iIndex);

  std::shared_ptr<Ui::CEditorCodeWidget>                 m_spUi;
  std::shared_ptr<CCodeWidgetTutorialStateSwitchHandler> m_spTutorialStateSwitchHandler;
  std::shared_ptr<CSettings>                             m_spSettings;
  tspProject                                             m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                        m_wpDbManager;
  std::weak_ptr<CScriptRunner>                           m_wpScriptRunner;
  QPointer<CFilteredEditorEditableFileModel>             m_pFilteredScriptModel;
  QPointer<QStandardItemModel>                           m_pDummyModel;
  QMetaObject::Connection                                m_debugFinishedConnection;
  bool                                                   m_bDebugging;
  bool                                                   m_bChangingIndex;
  QString                                                m_sLastCachedScript;
};

#endif // EDITORCODEWIDGET_H
