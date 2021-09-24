#ifndef CRESOURCEDETAILVIEW_H
#define CRESOURCEDETAILVIEW_H

#include "Systems/Project.h"
#include <QListView>
#include <memory>

class CResourceDetailView : public QListView
{
  Q_OBJECT

public:
  explicit CResourceDetailView(QWidget* pParent = nullptr);

  void Initialize(tspProject spProject);
  void DeInitilaze();

  void Expand(const QModelIndex& index);
  void Collapse(const QModelIndex& index);

  void SetReadOnly(bool bReadOnly);
  bool ReadOnly();

private:
  tspProject                                  m_spProject;
  bool                                        m_bReadOnly;
};

#endif // CRESOURCEDETAILVIEW_H
