#ifndef TAGSVIEW_H
#define TAGSVIEW_H

#include "Systems/DatabaseInterface/TagData.h"

#include <QFrame>

#include <functional>
#include <memory>
#include <type_traits>

class QPushButton;
namespace Ui {
  class CTagsView;
}

class CTagsView : public QFrame
{
  Q_OBJECT

public:
  explicit CTagsView(QWidget *parent = nullptr);
  ~CTagsView();

  using tfnAdded = std::function<void(QPushButton*, const QString&)>;
  using tfnRemoved = std::function<void(const QStringList&)>;
  void SetCallbacks(tfnAdded fnAdded, tfnRemoved fnRemoved);
  using tfnSort = std::function<void(std::vector<std::shared_ptr<SLockableTagData>>&)>;
  void SetSortFunction(tfnSort fnSort);

  void AddTags(const std::vector<std::shared_ptr<SLockableTagData>>& vspTags);
  void ClearTags();
  void RemoveTags(QStringList vsTags);

  const std::vector<std::shared_ptr<SLockableTagData>>& Tags() const;

private:

  std::unique_ptr<Ui::CTagsView>                 m_spUi;
  std::vector<std::shared_ptr<SLockableTagData>> m_vspTags;
  tfnAdded                                       m_fnAdded;
  tfnRemoved                                     m_fnRemoved;
  tfnSort                                        m_fnSort;
};

#endif // TAGSVIEW_H
