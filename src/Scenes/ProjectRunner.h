#ifndef PROJECTRUNNER_H
#define PROJECTRUNNER_H

#include <QObject>
#include <memory>

struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CProjectRunner : public QObject
{
  Q_OBJECT

public:
  explicit CProjectRunner(QObject* pParent = nullptr);
  ~CProjectRunner();

  void LoadProject(tspProject spProject);
  void UnloadProject();

private:
  tspProject                      m_spCurrentProject;
};

#endif // PROJECTRUNNER_H
