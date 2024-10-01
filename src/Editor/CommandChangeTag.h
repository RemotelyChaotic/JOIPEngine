#ifndef CCOMMANDCHANGETAG_H
#define CCOMMANDCHANGETAG_H

#include <QUndoCommand>
#include <memory>

class CDatabaseManager;

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

#endif // CCOMMANDCHANGETAG_H
