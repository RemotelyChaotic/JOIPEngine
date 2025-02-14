#ifndef CPROJECTSAVEGAMEMANAGER_H
#define CPROJECTSAVEGAMEMANAGER_H

#include <QJSEngine>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QVariant>

#include <map>
#include <memory>
#include <optional>

class CProjectScriptWrapper;
class CSceneMainScreen;

class IProjectSavegamePersistance
{
public:
  struct SSettingFile
  {
    QString     m_sResourceName;
    QVariantMap m_settings;
  };

  virtual std::optional<SSettingFile> Read(const QString& sProject, const QString& sContext) const = 0;
  virtual void Write(const QString& sProject, const QString& sContext, const SSettingFile& file) const = 0;

protected:
  virtual ~IProjectSavegamePersistance(){}
};

//----------------------------------------------------------------------------------------
//
class CProjectSavegameManager : public QObject
{
  Q_OBJECT
  Q_PROPERTY(CProjectScriptWrapper* project READ Project WRITE SetProject NOTIFY projectChanged)

public:
  explicit CProjectSavegameManager(QObject* pParent = nullptr);
  ~CProjectSavegameManager() override;

  CProjectScriptWrapper* Project() const;
  void SetProject(CProjectScriptWrapper* pProj);

public slots:
  QVariant load(const QString& sId, const QString& sContext);
  void removeData(const QString& sId, const QString& sContext);
  void store(const QString& sId, QVariant value, const QString& sContext);

signals:
  void achievementValueChanged(const QString& sId, QVariant value, QVariant oldValue);
  void projectChanged();

private:
  void AutoloadSaves();

  std::shared_ptr<IProjectSavegamePersistance>                 m_spPersistence;
  QPointer<CProjectScriptWrapper>                              m_pProject;
  std::map<QString, IProjectSavegamePersistance::SSettingFile> m_vSettings;
};

#endif // CPROJECTSAVEGAMEMANAGER_H
