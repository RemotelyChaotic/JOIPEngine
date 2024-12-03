#ifndef CPROJECTDIALOGMANAGER_H
#define CPROJECTDIALOGMANAGER_H

#include "Systems/DialogTree.h"
#include "Systems/Project.h"

#include <QObject>
#include <QPointer>

#include <tuple>
#include <vector>

class CProjectDialogManager : public QObject
{
  Q_OBJECT

public:
  CProjectDialogManager();
  ~CProjectDialogManager();

  void LoadProject(const tspProject& spProject);
  void UnloadProject();

  std::shared_ptr<CDialogNodeDialog> FindDialog(const QString& sId);
  std::vector<std::shared_ptr<CDialogNodeDialog>> FindDialog(const QRegExp& rx);
  std::vector<std::shared_ptr<CDialogNodeDialog>> FindDialogByTag(const QStringList& vsTags);

private:
  std::shared_ptr<CDialogNode>                                        m_spDataRootNode;
  std::vector<std::pair<QString, std::shared_ptr<CDialogNodeDialog>>> m_vspDialogsOnlyFlat;
};

//----------------------------------------------------------------------------------------
//
class CProjectDialogManagerWrapper : public QObject
{
  Q_OBJECT

public:
  CProjectDialogManagerWrapper(QPointer<QJSEngine> pEngine, std::weak_ptr<CProjectDialogManager> wpInstance);
  ~CProjectDialogManagerWrapper();

  Q_INVOKABLE QJSValue dialog(const QString& sId);
  Q_INVOKABLE QJSValue dialogFromRx(const QString& sId);
  Q_INVOKABLE QJSValue dialogFromTags(const QStringList& vsId);

private:
  std::weak_ptr<CProjectDialogManager> m_wpInstance;
  QPointer<QJSEngine> m_pEngine;
};

#endif // CPROJECTDIALOGMANAGER_H
