#ifndef CDIALOGueEDITORSORTFILTERPROXYMODEL_H
#define CDIALOGueEDITORSORTFILTERPROXYMODEL_H

#include <QCollator>
#include <QSortFilterProxyModel>

class CDialogueEditorSortFilterProxyModel : public QSortFilterProxyModel
{
public:
  CDialogueEditorSortFilterProxyModel(QObject* pParent = nullptr);
  ~CDialogueEditorSortFilterProxyModel() override;

  void setSourceModel(QAbstractItemModel* pSourceModel) override;

protected:
  bool filterAcceptsRow(int iSourceRow, const QModelIndex& sourceParent) const override;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private slots:
  void SlotResourceAdded();
  void SlotResourceRemoved();

private:
  QCollator                  m_collator;
};

#endif // CDIALOGueEDITORSORTFILTERPROXYMODEL_H
