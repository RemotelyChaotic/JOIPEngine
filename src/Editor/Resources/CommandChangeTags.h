#ifndef CCOMMANDCHANGETAGS_H
#define CCOMMANDCHANGETAGS_H

#include "Systems/Project.h"

#include <QStringList>
#include <QUndoCommand>
#include <functional>

class CDatabaseManager;

class CCommandAddTag : public QUndoCommand
{
public:
  CCommandAddTag(const QString& sProject, const QString& sResource,
                 const tspTag& spTag);
  ~CCommandAddTag();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QString                         m_sCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString                         m_sResource;
  tspTag                          m_spTag;
};

//----------------------------------------------------------------------------------------
//
class CCommandChangeTag : public QUndoCommand
{
public:
  CCommandChangeTag(const QString& sProject, const QString& sTag,
                    const QString& sNewCategory, const QString& sOldCategory,
                    const QString& sNewDescribtion, const QString& sOldDescribtion);
  ~CCommandChangeTag();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QString                         m_sCurrentProject;
  QString                         m_sTag;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString                         m_sNewCategory;
  QString                         m_sOldCategory;
  QString                         m_sNewDescribtion;
  QString                         m_sOldDescribtion;
};

//----------------------------------------------------------------------------------------
//
class CCommandRemoveTag : public QUndoCommand
{
public:
  CCommandRemoveTag(const QString& sProject, const QString& sResource,
                     const tspTag& spTag);
  ~CCommandRemoveTag();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QString                         m_sCurrentProject;
  std::weak_ptr<CDatabaseManager> m_wpDbManager;
  QString                         m_sResource;
  tspTag                          m_spTag;
};

#endif // CCOMMANDCHANGETAGS_H
