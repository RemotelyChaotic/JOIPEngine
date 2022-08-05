#ifndef CEOSSCRIPTEDITORVIEW_H
#define CEOSSCRIPTEDITORVIEW_H

#include <QPointer>
#include <QTreeView>

class CEosScriptModel;
class CEosScriptEditorView : public QTreeView
{
  Q_OBJECT
public:
  explicit CEosScriptEditorView(QWidget* pParent = nullptr);
  ~CEosScriptEditorView();

  void setModel(QAbstractItemModel* pModel);
  void ExpandAll();

signals:
  void SignalClickedOutside();

protected slots:
  void SlotModelReset();

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void focusInEvent(QFocusEvent* event) override;

private:
  void IterateItems(const QModelIndex& index,
                    const std::function<void(const QModelIndex&)>& fnToCall);

  QPointer<CEosScriptModel> m_pModel = nullptr;
};

#endif // CEOSSCRIPTEDITORVIEW_H
