#ifndef RESOURCETREEITEMMODEL_H
#define RESOURCETREEITEMMODEL_H

#include "Systems/Database/Project.h"
#include <QAbstractItemModel>
#include <QImage>
#include <QModelIndex>
#include <QPointer>
#include <QTimer>
#include <QVariant>
#include <map>
#include <memory>

class CDatabaseManager;
class CResourceTreeItem;
class QUndoStack;

class CResourceTreeItemModel : public QAbstractItemModel
{
  Q_OBJECT
  friend class CCommandChangeResourceData;

public:
  enum ECutomRoles
  {
    eSearchRole = Qt::UserRole,
    eLoadedIDRole = Qt::UserRole+1,
    eItemTypeRole = Qt::UserRole+2,
    eItemWarningRole = Qt::UserRole+3
  };

  explicit CResourceTreeItemModel(QPointer<QUndoStack> pUndoStack,
                                  QObject* pParent = nullptr);
  ~CResourceTreeItemModel() override;

  void InitializeModel(tspProject spProject);
  void DeInitializeModel();
  void SetCardIcon(const QImage& img);
  void SetLayoutIcon(const QImage& img);
  void SetIconSize(qint32 iValue);
  qint32 IconSize() const { return m_iIconSize; }

  tspProject Project() const { return m_spProject; };

  // read-only functions
  QVariant data(const QModelIndex& index, int iRole, int iColumnOverride);
  QVariant data(const QModelIndex& index, int iRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int iSection, Qt::Orientation orientation,
                      int iRole = Qt::DisplayRole) const override;
  QModelIndex index(int iRow, int iColumn,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  // write functions
  bool setData(const QModelIndex& index, const QVariant& value,
               qint32 iRole = Qt::EditRole) override;
  bool setHeaderData(qint32 iSection, Qt::Orientation orientation,
                     const QVariant& value, qint32 iRole = Qt::EditRole) override;
  bool insertColumns(qint32 iPosition, qint32 iColumns,
                     const QModelIndex& parent = QModelIndex()) override;
  bool removeColumns(qint32 iPosition, qint32 iColumns,
                     const QModelIndex& parent = QModelIndex()) override;
  bool insertRows(qint32 iPosition, qint32 iRows,
                  const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(qint32 iPosition, qint32 iRows,
                  const QModelIndex& parent = QModelIndex()) override;

  // convenience-functions
  bool IsResourceType(const QModelIndex& index = QModelIndex());
  bool IsFolderType(const QModelIndex& index = QModelIndex());
  QModelIndex IndexForResource(const tspResource& spResource);
  tspResource ResourceForIndex(const QModelIndex& idx);
  CResourceTreeItem* GetItem(const QModelIndex& index) const;

signals:
  void SignalProjectEdited();

public slots:
  void SlotProjectPropertiesEdited();

protected:
  void CheckChildResources(CResourceTreeItem* pParent);

private slots:
  void SlotResourceCheckerTimeout();
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);
  void SlotSceneDataChanged(qint32 iProjId, qint32 iId);

private:
  std::weak_ptr<CDatabaseManager>             m_wpDbManager;
  QPointer<QUndoStack>                        m_pUndoStack;
  CResourceTreeItem*                          m_pRootItem;
  std::map<EResourceType, CResourceTreeItem*> m_categoryMap;
  tspProject                                  m_spProject;
  QTimer                                      m_resourcecheckTimer;
  QString                                     m_sOldProjectLayoutResource;
  QString                                     m_sOldProjectTitleResource;
  QImage                                      m_cardIcon;
  QImage                                      m_layoutIcon;
  qint32                                      m_iIconSize;
};

#endif // RESOURCETREEITEMMODEL_H
