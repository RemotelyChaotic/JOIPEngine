#ifndef CCOMMANDCHANGEMODELVIAGUI_H
#define CCOMMANDCHANGEMODELVIAGUI_H

#include <QAbstractItemModel>
#include <QPointer>
#include <QUndoCommand>
#include <memory>

class CDialogEditorTreeModel;

class CCommandChangeModelViaGui : public QUndoCommand
{
public:
  CCommandChangeModelViaGui(const QVariant& oldValue, const QVariant& newValue,
                            const QString& sHeader,
                            const QStringList& vsPath,
                            QPointer<CDialogEditorTreeModel> pModel);
  ~CCommandChangeModelViaGui();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

private:
  void DoUndo(const QVariant& val);

  QPointer<CDialogEditorTreeModel> m_pModel;
  QStringList m_vsPath;
  QVariant m_oldValue;
  QVariant m_newValue;
  QString m_sHeader;
};

#endif // CCOMMANDCHANGEMODELVIAGUI_H
