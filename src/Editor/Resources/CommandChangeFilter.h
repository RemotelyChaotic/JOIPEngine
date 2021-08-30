#ifndef CCOMMANDCHANGEFILTER_H
#define CCOMMANDCHANGEFILTER_H

#include <QPointer>
#include <QUndoCommand>
#include <QLineEdit>

class CResourceTreeItemSortFilterProxyModel;
class CSearchWidget;

class CCommandChangeFilter : public QUndoCommand
{
public:
  CCommandChangeFilter(QPointer<CResourceTreeItemSortFilterProxyModel> m_pProxyModel,
                       QPointer<CSearchWidget> pSearchWidget,
                       QUndoCommand* pParent = nullptr);
  ~CCommandChangeFilter();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CResourceTreeItemSortFilterProxyModel> m_pProxyModel;
  QPointer<CSearchWidget> m_pSearchWidget;
  QString                m_sOldFilter;
  QString                m_sNewFilter;
};

#endif // CCOMMANDCHANGEFILTER_H
