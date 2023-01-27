#ifndef CEOSSCRIPTOVERLAYDELEGATE_H
#define CEOSSCRIPTOVERLAYDELEGATE_H

#include "EosScriptModelItem.h"
#include "Systems/JSON/JsonInstructionTypes.h"
#include "Widgets/OverlayBase.h"
#include <QPointer>

class CResourceTreeItemModel;

class CEosScriptOverlayDelegate : public COverlayBase
{
  Q_OBJECT
public:
  CEosScriptOverlayDelegate(QWidget* pParent = nullptr);
  ~CEosScriptOverlayDelegate() override;

  SItemIndexPath CurrentPath() const;
  tInstructionMapValue CurrentProperties() const;
  void Initialize(CResourceTreeItemModel* pEditorModel);
  void Show(const SItemIndexPath& path, const QString& sType,
            const tInstructionMapValue& props);

  bool IsForcedOpen() const;

public slots:
  void Climb() override;
  void Hide() override;
  void Resize() override;

signals:
  void SignalCurrentItemChanged();
  void SignalInvalidateItemChildren();

private:
  void ClearLayout();

  QPointer<CResourceTreeItemModel> m_pEditorModel;
  SItemIndexPath                   m_path;
  tInstructionMapValue             m_propsCopy;
};

#endif // CEOSSCRIPTOVERLAYDELEGATE_H
