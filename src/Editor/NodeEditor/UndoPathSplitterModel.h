#ifndef CUNDOPATHSPLITTERMODELWITHWIDGET_H
#define CUNDOPATHSPLITTERMODELWITHWIDGET_H

#include "UndoStackAwareModel.h"

#include "Systems/Nodes/PathSplitterModel.h"

class CUndoPathSplitterModel : public CPathSplitterModel,
                               public CUndoStackAwareModel
{
  Q_OBJECT

public:
  CUndoPathSplitterModel();
  ~CUndoPathSplitterModel() override;

protected:
  void SlotCustomTransitionChangedImpl(bool bEnabled, const QString& sResource) override;
  void SlotTransitionTypeChangedImpl(qint32 iType) override;
  void SlotTransitionLabelChangedImpl(PortIndex index, const QString& sLabelValue) override;

  void UndoRestore(QJsonObject const& p) override;
  void OnUndoStackSet() override {}

  bool                             m_bIsInUndoOperation = false;
};

//----------------------------------------------------------------------------------------
//
class CUndoPathSplitterModelWithWidget : public CPathSplitterModelWithWidget,
                                         public CUndoStackAwareModel
{
  Q_OBJECT
public:
  CUndoPathSplitterModelWithWidget();
  ~CUndoPathSplitterModelWithWidget() override;

protected:
  void SlotCustomTransitionChangedImpl(bool bEnabled, const QString& sResource) override;
  void SlotTransitionTypeChangedImpl(qint32 iType) override;
  void SlotTransitionLabelChangedImpl(PortIndex index, const QString& sLabelValue) override;

  void UndoRestore(QJsonObject const& p) override;
  void OnUndoStackSet() override;

  bool                             m_bIsInUndoOperation = false;
};

#endif // CUNDOPATHSPLITTERMODELWITHWIDGET_H
