#ifndef CCOMMANDCHANGEDialogueMODELVIAGUI_H
#define CCOMMANDCHANGEDialogueMODELVIAGUI_H

#include <QAbstractItemModel>
#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CDialogueEditorTreeModel;

class CCommandChangeDialogueModelViaGui : public QUndoCommand
{
public:
  CCommandChangeDialogueModelViaGui(const QVariant& oldValue, const QVariant& newValue,
                            const QString& sHeader,
                            const QStringList& vsPath,
                            QPointer<CDialogueEditorTreeModel> pModel);
  ~CCommandChangeDialogueModelViaGui();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

private:
  void DoUndo(const QVariant& val);

  QPointer<CDialogueEditorTreeModel> m_pModel;
  QStringList m_vsPath;
  QVariant m_oldValue;
  QVariant m_newValue;
  QString m_sHeader;
};

#endif // CCOMMANDCHANGEDialogueMODELVIAGUI_H
