#ifndef CCOMMANDCHANGEFETISHES_H
#define CCOMMANDCHANGEFETISHES_H

#include <QPointer>
#include <QStringList>
#include <QUndoCommand>
#include <functional>

class CCommandAddFetishes : public QUndoCommand
{
public:
  CCommandAddFetishes(QPointer<QWidget> pGuard,
                      std::function<void(QStringList)> fnAdd,
                      std::function<void(QStringList)> fnRemove,
                      QStringList vsKinks,
                      QUndoCommand* pParent = nullptr);
  ~CCommandAddFetishes();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QWidget>                m_pGuard;
  std::function<void(QStringList)> m_fnAdd;
  std::function<void(QStringList)> m_fnRemove;
  QStringList                      m_vsAddedKinks;
};

//----------------------------------------------------------------------------------------
//
class CCommandRemoveFetishes : public QUndoCommand
{
public:
  CCommandRemoveFetishes(QPointer<QWidget> pGuard,
                         std::function<void(QStringList)> fnAdd,
                         std::function<void(QStringList)> fnRemove,
                         QStringList vsKinks,
                         QUndoCommand* pParent = nullptr);
  ~CCommandRemoveFetishes();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<QWidget>                m_pGuard;
  std::function<void(QStringList)> m_fnAdd;
  std::function<void(QStringList)> m_fnRemove;
  QStringList                      m_vsRemovedKinks;
};

#endif // CCOMMANDCHANGEFETISHES_H
