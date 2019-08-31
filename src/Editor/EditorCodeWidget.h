#ifndef EDITORCODEWIDGET_H
#define EDITORCODEWIDGET_H

#include "EditorWidgetBase.h"
#include <QFileSystemWatcher>
#include <QPointer>
#include <memory>

class CDatabaseManager;
class CScriptHighlighter;
class CSettings;
namespace Ui {
  class CEditorCodeWidget;
}
struct SResource;
struct SScene;
typedef std::shared_ptr<SResource> tspResource;
typedef std::shared_ptr<SScene> tspScene;

struct SCachedMapItem
{
  SCachedMapItem() :
    m_watcher(), m_data(), m_bChanged(false)
  {}
  SCachedMapItem(const SCachedMapItem& other) :
    m_watcher(), m_data(other.m_data), m_bChanged(other.m_bChanged)
  {
    m_watcher.addPaths(other.m_watcher.directories() + other.m_watcher.files());
  }

  QFileSystemWatcher  m_watcher;
  QByteArray          m_data;
  bool                m_bChanged;
};

class CEditorCodeWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  explicit CEditorCodeWidget(QWidget* pParent = nullptr);
  ~CEditorCodeWidget() override;

  void Initialize() override;
  void LoadProject(tspProject spProject) override;
  void UnloadProject() override;
  void SaveProject() override;

  void LoadResource(tspResource spResource);

protected:
  void OnActionBarAboutToChange() override;
  void OnActionBarChanged() override;

protected slots:
  void on_pSceneComboBox_currentIndexChanged(qint32 iIndex);
  void on_pCodeEdit_textChanged();
  void SlotFileChanged(const QString& sPath);
  void SlotSceneAdded(qint32 iProjId, qint32 iId);
  void SlotSceneRenamed(qint32 iProjId, qint32 iId);
  void SlotSceneRemoved(qint32 iProjId, qint32 iId);

private:
  void AddNewScriptFile(tspScene spScene);
  QString FindSceneName(qint32 iId);
  QByteArray LoadScriptFile(const QString& sFile);
  void SetSceneScriptModifiedFlag(qint32 iId, bool bModified);

  std::unique_ptr<Ui::CEditorCodeWidget>     m_spUi;
  std::shared_ptr<CSettings>                 m_spSettings;
  tspProject                                 m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>            m_wpDbManager;
  QPointer<CScriptHighlighter>               m_pHighlighter;
  std::map<qint32, SCachedMapItem>           m_cachedScriptsMap;
  qint32                                     m_iLastIndex;
};

#endif // EDITORCODEWIDGET_H
