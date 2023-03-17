#ifndef CEOSSCRIPTEDITORVIEW_H
#define CEOSSCRIPTEDITORVIEW_H

#include "Widgets/Editor/EditorSearchBar.h"

#include <QPointer>
#include <QTreeView>

class CEditorSearchBar;
class CEosScriptModel;
class CEosScriptEditorView : public QTreeView
{
  Q_OBJECT
public:
  explicit CEosScriptEditorView(QWidget* pParent = nullptr);
  ~CEosScriptEditorView();

  void setModel(QAbstractItemModel* pModel) override;
  void ExpandAll();

signals:
  void SignalClickedOutside();
  void SignalContentsChange(qint32 iPos, qint32 iDel, qint32 iAdd);

protected slots:
  void SlotCreateContextMenu(QPoint p);
  void SlotSearchFilterChanged(CEditorSearchBar::ESearhDirection direction,
                               const QString& sText);
  void SlotModelReset();
  void SlotShowHideSearchFilter();
  void SlotSearchAreaHidden();

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void focusInEvent(QFocusEvent* event) override;

private:
  void IterateItems(const QModelIndex& index,
                    const std::function<void(const QModelIndex&)>& fnToCall);

  QPointer<CEosScriptModel> m_pModel;
  QPointer<CEditorSearchBar>m_pSearchBar;
};

#endif // CEOSSCRIPTEDITORVIEW_H
