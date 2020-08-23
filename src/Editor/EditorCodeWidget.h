#ifndef EDITORCODEWIDGET_H
#define EDITORCODEWIDGET_H

#include "EditorWidgetBase.h"
#include <QPointer>
#include <memory>

class CBackgroundSnippetOverlay;
class CIconSnippetOverlay;
class CMetronomeSnippetOverlay;
class CResourceSnippetOverlay;
class CTextSnippetOverlay;
class CTimerSnippetOverlay;
class CThreadSnippetOverlay;
class CDatabaseManager;
class CScriptHighlighter;
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

  void LoadResource(tspResource spResource);

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

protected slots:
  void on_pResourceComboBox_currentIndexChanged(qint32 iIndex);
  void on_pCodeEdit_textChanged();
  void SlotDebugStart();
  void SlotDebugStop();
  void SlotFileChangedExternally(const QString& sName);
  void SlotInsertGeneratedCode(const QString& sCode);
  void SlotRowsInserted(const QModelIndex& parent, int iFirst, int iLast);
  void SlotRowsRemoved(const QModelIndex& parent, int iFirst, int iLast);

private:
  QString FindSceneName(qint32 iId);

  std::unique_ptr<Ui::CEditorCodeWidget>     m_spUi;
  std::unique_ptr<CBackgroundSnippetOverlay> m_spBackgroundSnippetOverlay;
  std::unique_ptr<CIconSnippetOverlay>       m_spIconSnippetOverlay;
  std::unique_ptr<CMetronomeSnippetOverlay>  m_spMetronomeSnippetOverlay;
  std::unique_ptr<CResourceSnippetOverlay>   m_spResourceSnippetOverlay;
  std::unique_ptr<CTextSnippetOverlay>       m_spTextSnippetOverlay;
  std::unique_ptr<CTimerSnippetOverlay>      m_spTimerSnippetOverlay;
  std::unique_ptr<CThreadSnippetOverlay>     m_spThreadSnippetOverlay;
  std::shared_ptr<CSettings>                 m_spSettings;
  tspProject                                 m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>            m_wpDbManager;
  std::weak_ptr<CScriptRunner>               m_wpScriptRunner;
  QPointer<QStandardItemModel>               m_pDummyModel;
  qint32                                     m_iLastIndex;
};

#endif // EDITORCODEWIDGET_H
