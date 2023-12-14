#ifndef SCRIPTEDITORMODEL_H
#define SCRIPTEDITORMODEL_H

#include "Systems/Project.h"
#include <QFileSystemWatcher>
#include <QStandardItemModel>

#include <QPointer>
#include <QWidget>
#include <map>
#include <memory>

class CDatabaseManager;

//----------------------------------------------------------------------------------------
//
struct SCachedMapItem
{
  SCachedMapItem() :
    m_vspScenes(), m_spWatcher(std::make_shared<QFileSystemWatcher>()), m_data(), m_sId(),
    m_sHighlightDefinition("JavaScript"), m_sScriptType("js"),
    m_bChanged(false), m_bIgnoreNextModification(false), m_bAllreadyAsked(false), m_bInitialized(false)
  {}
  SCachedMapItem(const SCachedMapItem& other) :
    m_vspScenes(other.m_vspScenes),
    m_spWatcher(other.m_spWatcher), m_data(other.m_data),
    m_sId(other.m_sId),
    m_sHighlightDefinition(other.m_sHighlightDefinition),
    m_sScriptType(other.m_sScriptType),
    m_bChanged(other.m_bChanged),
    m_bIgnoreNextModification(other.m_bIgnoreNextModification),
    m_bAllreadyAsked(other.m_bAllreadyAsked),
    m_bInitialized(other.m_bInitialized)
  {
  }

  tvspScene                            m_vspScenes;
  std::shared_ptr<QFileSystemWatcher>  m_spWatcher;
  QByteArray                           m_data;
  QString                              m_sId;
  QString                              m_sHighlightDefinition;
  QString                              m_sScriptType;
  bool                                 m_bChanged;
  bool                                 m_bIgnoreNextModification;
  bool                                 m_bAllreadyAsked;
  bool                                 m_bInitialized;
};

//----------------------------------------------------------------------------------------
//
class CEditorEditableFileModel : public QStandardItemModel
{
  Q_OBJECT

public:
  explicit CEditorEditableFileModel(QWidget* pParent = nullptr);
  ~CEditorEditableFileModel() override;

  static void InitScript(QIODevice& file, const QString& sType);

  SCachedMapItem* CachedScript(const QString& sName);
  QString CachedScriptName(qint32 iName);
  void InitializeModel(tspProject spProject);
  void DeInitializeModel();
  void SerializeProject();
  qint32 ScriptIndex(const QString& sName);
  void SetReloadFileWithoutQuestion(bool bReload);
  void SetSceneScriptModifiedFlag(const QString& sName, bool bModified);

  Q_INVOKABLE QModelIndex index(int iRow, int iCol, const QModelIndex& parent = QModelIndex()) const override;
  Q_INVOKABLE int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  Q_INVOKABLE int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  Q_INVOKABLE QVariant data(const QModelIndex& index, int iRole = Qt::DisplayRole) const override;
  Q_INVOKABLE Qt::ItemFlags flags(const QModelIndex &index) const override;

signals:
  void SignalFileChangedExternally(const QString& sName);
  void SignalProjectEdited();

public slots:
  void SlotProjectPropertiesEdited();

private slots:
  void SlotFileChanged(const QString& sPath);
  void SlotResourceAdded(qint32 iProjId, const QString& sName);
  void SlotResourceRemoved(qint32 iProjId, const QString& sName);
  void SlotResourceRenamed(qint32 iProjId, const QString& sOldName,const QString& sName);
  void SlotSceneDatachanged(qint32 iProjId, qint32 iId);
  void SlotSceneRenamed(qint32 iProjId, qint32 iId);
  void SlotSceneRemoved(qint32 iProjId, qint32 iId);

private:
  void AddResourceTo(tspResource spResource, std::map<QString, SCachedMapItem>& mpToAddTo);
  void LoadScriptFile(const QString& sName);

  std::weak_ptr<CDatabaseManager>             m_wpDbManager;
  tspProject                                  m_spProject;
  QPointer<QWidget>                           m_pParentWidget;
  std::map<QString, SCachedMapItem>           m_cachedScriptsMap;
  QString                                     m_sOldProjectLayoutResource;
  bool                                        m_bReloadFileWithoutQuestion;
};

#endif // SCRIPTEDITORMODEL_H
