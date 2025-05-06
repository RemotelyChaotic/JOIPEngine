#ifndef CACHIEVEMENTWIDGET_H
#define CACHIEVEMENTWIDGET_H

#include "Systems/DatabaseInterface/SaveData.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QLineEdit>
#include <QPointer>
#include <QSpinBox>
#include <QTextEdit>
#include <QWidget>

class CShortcutButton;
class CResourceTreeItemModel;
class CSelectableResourceLabel;
typedef std::shared_ptr<struct SProject> tspProject;

class CAchievementWidget : public QFrame
{
  Q_OBJECT

public:
  explicit CAchievementWidget(const tspProject& spProj, QPointer<QAbstractItemModel> pResourceModel,
                              QWidget *parent = nullptr);
  ~CAchievementWidget() override;

  static const char c_sAchievementItemRootWidgetProperty[];

  void SetAchievementData(const SSaveDataData& saveData);
  const SSaveDataData& AchievementData() const;

signals:
  void SignalAchievementChanged(const SSaveDataData& oldData, const SSaveDataData& newData);
  void SignalRemove();

private:
  void EmitParamsChanged();

  tspProject m_spCurrentProject;
  QPointer<CSelectableResourceLabel> m_pButtonIcon;
  QPointer<QLineEdit> m_pLineEditHeader;
  QPointer<CShortcutButton> m_pRemoveButton;
  QPointer<QComboBox> m_pComboType;
  QPointer<QSpinBox> m_pSpinCounter;
  QPointer<QTextEdit> m_pTextEdit;
  SSaveDataData m_saveData;
};

#endif // CACHIEVEMENTWIDGET_H
