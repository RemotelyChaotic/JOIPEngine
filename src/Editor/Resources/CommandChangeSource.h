#ifndef CCOMMANDCHANGESOURCE_H
#define CCOMMANDCHANGESOURCE_H

#include "Systems/Project.h"
#include <QUndoCommand>
#include <memory>

class CDatabaseManager;

class CCommandChangeSource : public QUndoCommand
{
public:
  CCommandChangeSource(const tspProject& spCurrentProject,
                       const QString& sNameOfResource,
                       const QUrl& sOldSource,
                       const QUrl& sNewSource,
                       QUndoCommand* pParent = nullptr);
  ~CCommandChangeSource();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  tspProject                      m_spCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString m_sNameOfResource;
  QUrl m_sOldSource;
  QUrl m_sNewSource;

private:
  void DoUndoRedo(const QUrl& sSource);
};

#endif // CCOMMANDCHANGESOURCE_H
