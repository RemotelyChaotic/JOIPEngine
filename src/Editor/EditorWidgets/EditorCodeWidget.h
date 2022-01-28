#ifndef EDITORCODEWIDGET_H
#define EDITORCODEWIDGET_H

#include "EditorWidgetBase.h"
#include "ui_EditorCodeWidget.h"
#include "ui_EditorActionBar.h"
#include <QPointer>
#include <memory>

class CBackgroundSnippetOverlay;
class CCodeWidgetTutorialStateSwitchHandler;
class CIconSnippetOverlay;
class CMetronomeSnippetOverlay;
class CNotificationSnippetOverlay;
class CResourceSnippetOverlay;
class CTextSnippetOverlay;
class CTimerSnippetOverlay;
class CThreadSnippetOverlay;
class CDatabaseManager;
class CEditorHighlighter;
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
  void SlotInsertGeneratedCode(const QString& sCode);
  void SlotRowsInserted(const QModelIndex& parent, int iFirst, int iLast);
  void SlotRowsRemoved(const QModelIndex& parent, int iFirst, int iLast);
  void SlotUndoForScriptContentAdded();

private:
  void ReloadEditor(qint32 iIndex);
  void SetButtonsBasedOnScript(const QString& sScriptType);

  std::unique_ptr<CBackgroundSnippetOverlay>             m_spBackgroundSnippetOverlay;
  std::unique_ptr<CIconSnippetOverlay>                   m_spIconSnippetOverlay;
  std::unique_ptr<CMetronomeSnippetOverlay>              m_spMetronomeSnippetOverlay;
  std::unique_ptr<CNotificationSnippetOverlay>           m_spNotificationSnippetOverlay;
  std::unique_ptr<CResourceSnippetOverlay>               m_spResourceSnippetOverlay;
  std::unique_ptr<CTextSnippetOverlay>                   m_spTextSnippetOverlay;
  std::unique_ptr<CTimerSnippetOverlay>                  m_spTimerSnippetOverlay;
  std::unique_ptr<CThreadSnippetOverlay>                 m_spThreadSnippetOverlay;
  std::shared_ptr<Ui::CEditorCodeWidget>                 m_spUi;
  std::shared_ptr<CCodeWidgetTutorialStateSwitchHandler> m_spTutorialStateSwitchHandler;
  std::shared_ptr<CSettings>                             m_spSettings;
  tspProject                                             m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                        m_wpDbManager;
  std::weak_ptr<CScriptRunner>                           m_wpScriptRunner;
  QPointer<QStandardItemModel>                           m_pDummyModel;
  QMetaObject::Connection                                m_debugFinishedConnection;
  bool                                                   m_bDebugging;
  bool                                                   m_bChangingIndex;
  QString                                                m_sLastCachedScript;
};

#endif // EDITORCODEWIDGET_H
