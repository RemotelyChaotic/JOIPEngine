#include "ScriptEditorCompleterModel.h"
#include "Application.h"

#include "Systems/DatabaseManager.h"

#include <QDebug>

IScriptCompleterFileProcessor::IScriptCompleterFileProcessor(){}
IScriptCompleterFileProcessor::~IScriptCompleterFileProcessor(){}

//----------------------------------------------------------------------------------------
//
CScriptEditorCompleterModel::CScriptEditorCompleterModel(QObject* pParent) :
  QAbstractItemModel(pParent)
{
}
CScriptEditorCompleterModel::~CScriptEditorCompleterModel() = default;

//----------------------------------------------------------------------------------------
//
qint32 CScriptEditorCompleterModel::ColumnForType(const QString& sType)
{
  if (auto it = m_processorRegistry.find(sType); m_processorRegistry.end() != it)
  {
    return it->second.first;
  }
  return -1;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptCompleterFileProcessor>
CScriptEditorCompleterModel::FileProcessor(const QString& sType) const
{
  if (auto it = m_processorRegistry.find(sType); m_processorRegistry.end() != it)
  {
    return it->second.second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleterModel::ProcessLine(const QString& sType, const QString& sLine)
{
  auto spProcessor = FileProcessor(sType);
  auto iType = ColumnForType(sType);
  if (nullptr != spProcessor)
  {
    beginResetModel();
    spProcessor->ProcessLine(iType, sLine, &m_data);
    endResetModel();
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleterModel::RegisterFileProcessor(
    const QString& sFileType, std::shared_ptr<IScriptCompleterFileProcessor> spProcessor)
{
  m_processorRegistry.insert({sFileType, {GetNewColumn(), spProcessor}});
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleterModel::SetProject(tspProject spProject)
{
  beginResetModel();
  m_spProject = spProject;
  BuildModel();
  endResetModel();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CScriptEditorCompleterModel::index(int row, int column,
                                               const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  if (0 > column || static_cast<qint32>(m_data.m_vsAllData.size()) <= column)
  {
    return QModelIndex();
  }
  auto it = m_data.m_vsAllData.find(column);
  if (m_data.m_vsAllData.end() == it)
  {
    return QModelIndex();
  }
  if (0 > row || row >= static_cast<qint32>(it->second.size()))
  {
    return QModelIndex();
  }
  return createIndex(row, column);
}

//----------------------------------------------------------------------------------------
//
QModelIndex CScriptEditorCompleterModel::parent(const QModelIndex& child) const
{
  Q_UNUSED(child)
  return QModelIndex();
}

//----------------------------------------------------------------------------------------
//
QModelIndex CScriptEditorCompleterModel::sibling(int iRow, int iColumn, const QModelIndex& idx) const
{
  if (!idx.isValid() || 0 > iColumn || iRow > 0)
  {
    return QModelIndex();
  }

  auto it = m_data.m_vsAllData.find(iColumn);
  if (m_data.m_vsAllData.end() == it)
  {
    return QModelIndex();
  }

  if (iRow >= static_cast<qint32>(it->second.size()))
  {
    return QModelIndex();
  }

  return createIndex(iRow, iColumn);
}

//----------------------------------------------------------------------------------------
//
int CScriptEditorCompleterModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) { return 0; }
  auto it = m_data.m_vsAllData.find(parent.column());
  if (m_data.m_vsAllData.end() == it)
  {
    return 0;
  }
  return static_cast<qint32>(it->second.size());
}

//----------------------------------------------------------------------------------------
//
int CScriptEditorCompleterModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid()) { return 0; }
  return static_cast<qint32>(m_data.m_vsAllData.size());
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptEditorCompleterModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || 0 > index.row() || 0 > index.column())
  {
    return QVariant();
  }

  auto it = m_data.m_vsAllData.find(index.column());
  if (m_data.m_vsAllData.end() == it)
  {
    return QVariant();
  }

  if (index.row() >= static_cast<qint32>(it->second.size()))
  {
    return QVariant();
  }

  if (Qt::DisplayRole == role || Qt::EditRole == role)
  {
    auto itVal = it->second.begin();
    std::advance(itVal, static_cast<size_t>(index.row()));
    return *itVal;
  }

  return QVariant();
}

//----------------------------------------------------------------------------------------
//
Qt::ItemFlags CScriptEditorCompleterModel::flags(const QModelIndex &index) const
{
  return (QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren) & ~Qt::ItemIsEditable;
}

//----------------------------------------------------------------------------------------
//
void CScriptEditorCompleterModel::BuildModel()
{
  if (nullptr != m_spProject)
  {
    if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
    {
      QReadLocker pl(&m_spProject->m_rwLock);
      for (const auto& [_, spRes] : m_spProject->m_baseData.m_spResourcesMap)
      {
        QReadLocker rl(&spRes->m_rwLock);
        if (EResourceType::eScript == spRes->m_type._to_integral() ||
            EResourceType::eLayout == spRes->m_type._to_integral())
        {
          const QString sFilePath = spRes->ResourceToAbsolutePath();
          QFile file(sFilePath);
          if (file.open(QIODevice::ReadOnly | QIODevice::Text))
          {
            const QString sType = QFileInfo(sFilePath).suffix();
            auto it = m_processorRegistry.find(sType);
            if (m_processorRegistry.end() != it)
            {
              QTextStream stream(&file);
              stream.setCodec("UTF-8");
              it->second.second->ProcessFile(it->second.first, stream, &m_data);
            }
          }
          else
          {
            qWarning() << "Registered script resource could not be opened.";
          }
        }
      }
    }
  }
  else
  {
    m_data.m_vsAllData.clear();
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CScriptEditorCompleterModel::GetNewColumn()
{
  return m_iColumns++;
}
