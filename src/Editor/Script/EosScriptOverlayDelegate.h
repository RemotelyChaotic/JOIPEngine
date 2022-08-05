#ifndef CEOSSCRIPTOVERLAYDELEGATE_H
#define CEOSSCRIPTOVERLAYDELEGATE_H

#include "Widgets/OverlayBase.h"
#include <QPointer>

class CEosScriptModelItem;
class CResourceTreeItemModel;

class CEosScriptOverlayDelegate : public COverlayBase
{
  Q_OBJECT
public:
  CEosScriptOverlayDelegate(QWidget* pParent = nullptr);
  ~CEosScriptOverlayDelegate() override;

  void Initialize(CResourceTreeItemModel* pEditorModel);
  void Show(CEosScriptModelItem* pItem);

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
  CEosScriptModelItem*             m_pItem;
};

#endif // CEOSSCRIPTOVERLAYDELEGATE_H
