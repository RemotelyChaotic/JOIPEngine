#ifndef CCODEDISPLAYEOSEDITORIMPL_H
#define CCODEDISPLAYEOSEDITORIMPL_H

#include "ICodeDisplayWidgetImpl.h"

#include <QTreeView>

class CEosScriptModel;
class CEosScriptEditorView;
class CEosScriptOverlayDelegate;
class CEosSortFilterProxyModel;
class CJsonInstructionSetParser;

//----------------------------------------------------------------------------------------
//
class CCodeDisplayEosEditorImpl : public QObject, public ICodeDisplayWidgetImpl
{
  Q_OBJECT
public:
  CCodeDisplayEosEditorImpl(QPointer<CEosScriptEditorView> pTarget);
  ~CCodeDisplayEosEditorImpl() override;

  void Initialize(CEditorModel* pEditorModel) override;

  void Clear() override;
  void ExecutionError(QString sException, qint32 iLine, QString sStack) override;
  void InsertGeneratedCode(const QString& sCode) override;
  void ResetWidget() override;
  void SetContent(const QString& sContent) override;
  void SetHighlightDefinition(const QString& sType) override;
  void HideButtons(Ui::CEditorActionBar* pActionBar) override;
  void ShowButtons(Ui::CEditorActionBar* pActionBar) override;
  void Update() override;

  QString GetCurrentText() const override;

protected slots:
  void CurrentItemChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void CurrentItemDataChanged();
  void CurrentItemInvalidated();
  void SlotItemModelDataChanged(const QModelIndex& begin, const QModelIndex& end);
  void SlotAddAudio();
  void SlotAddChoice();
  void SlotAddDisable();
  void SlotAddEnable();
  void SlotAddEnd();
  void SlotAddEval();
  void SlotAddGoto();
  void SlotAddIf();
  void SlotAddImage();
  void SlotAddNotificationRemove();
  void SlotAddNotificationCreate();
  void SlotAddPrompt();
  void SlotAddSay();
  void SlotAddTimer();
  void SlotClickedOutsideView();
  void SlotRemoveInstruction();

private:
  std::unique_ptr<CJsonInstructionSetParser>   m_spEosParser;
  QPointer<CEosScriptEditorView>               m_pTarget;
  QPointer<CEosScriptOverlayDelegate>          m_pEditorDelegate;
  QPointer<CEosSortFilterProxyModel>           m_pProxy;
  QPointer<CEosScriptModel>                    m_pJsonParserModel;
  QPointer<CEditorModel>                       m_pEditorModel;
  bool                                         m_bEnableDelegateUpdate = true;
};

#endif // CCODEDISPLAYEOSEDITORIMPL_H
