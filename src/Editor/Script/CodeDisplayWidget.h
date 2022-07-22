#ifndef CODEDISPLAYWIDGET_H
#define CODEDISPLAYWIDGET_H

#include <QPointer>
#include <QWidget>

#include <memory>

class CBackgroundSnippetOverlay;
class CEditorModel;
class CIconSnippetOverlay;
class CMetronomeSnippetOverlay;
class CNotificationSnippetOverlay;
class CResourceSnippetOverlay;
class CResourceTreeItemModel;
class CScriptEditorModel;
class CTextSnippetOverlay;
class CTimerSnippetOverlay;
class CThreadSnippetOverlay;
namespace Ui {
  class CCodeDisplayWidget;
  class CEditorActionBar;
}
class QUndoStack;
typedef std::shared_ptr<struct SProject> tspProject;

class CCodeDisplayWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CCodeDisplayWidget(QWidget* pParent = nullptr);
  ~CCodeDisplayWidget();

  void Initialize(QPointer<CEditorModel> pEditorModel,
                  QPointer<CScriptEditorModel> pScriptEditorModel,
                  QPointer<CResourceTreeItemModel> pResourceTreeModel,
                  QPointer<QUndoStack> pUndoStack);
  void LoadProject(tspProject spProject);
  void UnloadProject();
  void SaveProject();
  void OnActionBarAboutToChange(std::unique_ptr<Ui::CEditorActionBar>* pspUiActionBar);
  void OnActionBarChanged(std::unique_ptr<Ui::CEditorActionBar>* pspUiActionBar);

  void Clear();
  void ResetWidget();
  void SetContent(const QString& sContent);
  void SetHighlightDefinition(const QString& sType);
  void SetScriptType(const QString& sScriptType);
  void Update();

  QString GetCurrentText() const;

public slots:
  void SlotExecutionError(QString sException, qint32 iLine, QString sStack);
  void SlotShowOverlay();

signals:
  void SignalContentsChange(qint32 iPos, qint32 iDel, qint32 iAdd);

protected slots:
  void SlotInsertGeneratedCode(const QString& sCode);
  void SlotUndoForScriptContentAdded();

private:
  std::unique_ptr<Ui::CCodeDisplayWidget>                m_spUi;
  std::unique_ptr<Ui::CEditorActionBar>*                 m_pspUiActionBar = nullptr;

  std::unique_ptr<CBackgroundSnippetOverlay>             m_spBackgroundSnippetOverlay;
  std::unique_ptr<CIconSnippetOverlay>                   m_spIconSnippetOverlay;
  std::unique_ptr<CMetronomeSnippetOverlay>              m_spMetronomeSnippetOverlay;
  std::unique_ptr<CNotificationSnippetOverlay>           m_spNotificationSnippetOverlay;
  std::unique_ptr<CResourceSnippetOverlay>               m_spResourceSnippetOverlay;
  std::unique_ptr<CTextSnippetOverlay>                   m_spTextSnippetOverlay;
  std::unique_ptr<CTimerSnippetOverlay>                  m_spTimerSnippetOverlay;
  std::unique_ptr<CThreadSnippetOverlay>                 m_spThreadSnippetOverlay;

  QPointer<CEditorModel>                                 m_pEditorModel;
  QPointer<CScriptEditorModel>                           m_pScriptEditorModel;
  QPointer<CResourceTreeItemModel>                       m_pResourceTreeModel;
  QPointer<QUndoStack>                                   m_pUndoStack;
  bool                                                   m_pInitialized = false;
};

#endif // CODEDISPLAYWIDGET_H
