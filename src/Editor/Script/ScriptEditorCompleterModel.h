#ifndef CSCRIPTEDITORCOMPLETERMODEL_H
#define CSCRIPTEDITORCOMPLETERMODEL_H

#include "Systems/Project.h"

#include <QAbstractListModel>
#include <QTextStream>

#include <map>
#include <set>
#include <type_traits>

struct SCompleterModelData
{
  std::map<int, std::set<QString>> m_vsAllData;
};

//----------------------------------------------------------------------------------------
//
class IScriptCompleterFileProcessor
{
public:
  IScriptCompleterFileProcessor();

  virtual bool IsEndOfWordChar(QChar c) const = 0;
  virtual void ProcessFile(int destRole, QTextStream& data, SCompleterModelData* pOutData) = 0;
  virtual void ProcessLine(int destRole, const QString& sLine, SCompleterModelData* pOutData) = 0;

protected:
  ~IScriptCompleterFileProcessor();
};

//----------------------------------------------------------------------------------------
//
class CScriptEditorCompleterModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit CScriptEditorCompleterModel(QObject* pParent);
  ~CScriptEditorCompleterModel() override;

  qint32 ColumnForType(const QString& sType);
  std::shared_ptr<IScriptCompleterFileProcessor> FileProcessor(const QString& sType) const;

  template<typename T, typename = std::enable_if_t<std::is_base_of_v<IScriptCompleterFileProcessor, T>>>
  void RegisterFileProcessor(const QString& sFileType)
  {
    m_processorRegistry.insert({sFileType, { GetNewColumn(), std::make_shared<T>() } });
  }
  void RegisterFileProcessor(const QString& sFileType, std::shared_ptr<IScriptCompleterFileProcessor> spProcessor);

  void SetProject(tspProject spProject);

  QModelIndex index(int row, int column,
                            const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex sibling(int iRow, int iColumn, const QModelIndex& idx) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
  void BuildModel();
  qint32 GetNewColumn();

  std::map<QString, std::pair<int, std::shared_ptr<IScriptCompleterFileProcessor>>> m_processorRegistry;
  tspProject m_spProject;
  SCompleterModelData m_data;
  qint32 m_iColumns = 0;
};

#endif // CSCRIPTEDITORCOMPLETERMODEL_H
