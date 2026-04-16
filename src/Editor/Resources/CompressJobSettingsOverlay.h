#ifndef COMPRESSJOBSETTINGSOVERLAY_H
#define COMPRESSJOBSETTINGSOVERLAY_H

#include "Editor/Resources/ResourceTreeItemSortFilterProxyModel.h"

#include "Widgets/OverlayBase.h"

#include <QPointer>

#include <memory>
#include <set>

class CResourceTreeItemModel;
namespace Ui {
  class CCompressJobSettingsOverlay;
}

class CCompressResourceModel : public CResourceTreeItemSortFilterProxyModel
{
public:
  explicit CCompressResourceModel(QObject* pParent = nullptr);
  ~CCompressResourceModel() override;

  const std::set<QString>& CheckedResources() const;
  void Reset();

  QVariant data(const QModelIndex& proxyIndex, qint32 role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool setData(const QModelIndex& index, const QVariant &value, qint32 role = Qt::EditRole) override;

private:
  std::set<QString> m_checked;
};

//----------------------------------------------------------------------------------------
//
class CCompressJobSettingsOverlay : public COverlayBase
{
  Q_OBJECT

public:
  struct SCompressJobSettings
  {
    qint32 m_iCompression;
    std::set<QString> m_resources;
  };

  explicit CCompressJobSettingsOverlay(QWidget* pParent = nullptr);
  ~CCompressJobSettingsOverlay();

public slots:
  void Climb() override;
  void Hide() override;
  void Show(const tspProject& spProject, QPointer<CResourceTreeItemModel> pModel);
  void Resize() override;

signals:
  void SignalJobSettingsConfirmed(const SCompressJobSettings& settings);

protected slots:
  void on_pFilter_SignalFilterChanged(const QString& sText);
  void on_pConfirmButton_clicked();
  void on_pCancelButton_clicked();

private:
  std::unique_ptr<Ui::CCompressJobSettingsOverlay> m_spUi;
  QPointer<CCompressResourceModel> m_pProxyModel;
};

Q_DECLARE_METATYPE(CCompressJobSettingsOverlay::SCompressJobSettings)
Q_DECLARE_METATYPE(std::set<QString>)

#endif // COMPRESSJOBSETTINGSOVERLAY_H
