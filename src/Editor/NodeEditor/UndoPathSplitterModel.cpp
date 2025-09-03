#include "UndoPathSplitterModel.h"

#include "CommandNodeEdited.h"

#include <QJsonArray>

CUndoPathSplitterModel::CUndoPathSplitterModel() :
  CPathSplitterModel(),
  CUndoStackAwareModel()
{
}
CUndoPathSplitterModel::~CUndoPathSplitterModel() = default;

//----------------------------------------------------------------------------------------
//
void CUndoPathSplitterModel::SlotCustomTransitionChangedImpl(bool bEnabled, const QString& sResource)
{
  QJsonObject oldState = save();
  CPathSplitterModel::SlotCustomTransitionChangedImpl(bEnabled, sResource);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoPathSplitterModel::SlotTransitionTypeChangedImpl(qint32 iType)
{
  QJsonObject oldState = save();
  CPathSplitterModel::SlotTransitionTypeChangedImpl(iType);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoPathSplitterModel::SlotTransitionLabelChangedImpl(PortIndex index, const QString& sLabelValue)
{
  QJsonObject oldState = save();
  CPathSplitterModel::SlotTransitionLabelChangedImpl(index, sLabelValue);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoPathSplitterModel::UndoRestore(QJsonObject const& p)
{
  m_bIsInUndoOperation = true;
  QJsonValue v = p["transitonType"];
  if (!v.isUndefined())
  {
    qint32 iValue = v.toInt();
    if (-1 < iValue && static_cast<qint32>(ESceneTransitionType::_size()) > iValue)
    {
      m_transitonType = ESceneTransitionType::_from_integral(iValue);
    }
  }
  QJsonValue arr = p["vsLabelNames"];
  if (!arr.isUndefined())
  {
    size_t i = 0;
    for (QJsonValue val : arr.toArray())
    {
      if (m_vsLabelNames.size() > i)
      {
        const QString sLabel = val.toString();
        m_vsLabelNames[i] = sLabel;
        i++;
      }
    }
  }
  v = p["bCustomLayoutEnabled"];
  bool bCustomLayoutEnabled = false;
  if (!v.isUndefined())
  {
    bCustomLayoutEnabled = v.toBool(false);
  }
  v = p["sCustomLayout"];
  QString sCustomLayout = QString();
  if (!v.isUndefined())
  {
    sCustomLayout = v.toString(QString());
  }
  SlotCustomTransitionChanged(bCustomLayoutEnabled, sCustomLayout);
  m_bIsInUndoOperation = false;
}

//----------------------------------------------------------------------------------------
//
CUndoPathSplitterModelWithWidget::CUndoPathSplitterModelWithWidget() :
  CPathSplitterModelWithWidget(),
  CUndoStackAwareModel()
{
}
CUndoPathSplitterModelWithWidget::~CUndoPathSplitterModelWithWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CUndoPathSplitterModelWithWidget::SlotCustomTransitionChangedImpl(bool bEnabled,
                                                                       const QString& sResource)
{
  QJsonObject oldState = save();
  CPathSplitterModelWithWidget::SlotCustomTransitionChangedImpl(bEnabled, sResource);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoPathSplitterModelWithWidget::SlotTransitionTypeChangedImpl(qint32 iType)
{
  QJsonObject oldState = save();
  CPathSplitterModelWithWidget::SlotTransitionTypeChangedImpl(iType);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoPathSplitterModelWithWidget::SlotTransitionLabelChangedImpl(PortIndex index, const QString& sLabelValue)
{
  QJsonObject oldState = save();
  CPathSplitterModelWithWidget::SlotTransitionLabelChangedImpl(index, sLabelValue);
  QJsonObject newState = save();

  if (nullptr != UndoStack() && !m_bIsInUndoOperation)
  {
    UndoStack()->push(new CCommandNodeEdited(m_pScene, m_id, oldState, newState));
  }
}

//----------------------------------------------------------------------------------------
//
void CUndoPathSplitterModelWithWidget::UndoRestore(QJsonObject const& p)
{
  m_bIsInUndoOperation = true;
  QJsonValue v = p["transitonType"];
  if (!v.isUndefined())
  {
    qint32 iValue = v.toInt();
    if (-1 < iValue && static_cast<qint32>(ESceneTransitionType::_size()) > iValue)
    {
      m_transitonType = ESceneTransitionType::_from_integral(iValue);
    }
  }
  QJsonValue arr = p["vsLabelNames"];
  if (!arr.isUndefined())
  {
    size_t i = 0;
    for (QJsonValue val : arr.toArray())
    {
      if (m_vsLabelNames.size() > i)
      {
        const QString sLabel = val.toString();
        m_vsLabelNames[i] = sLabel;
        i++;
      }
    }
  }
  v = p["bCustomLayoutEnabled"];
  bool bCustomLayoutEnabled = false;
  if (!v.isUndefined())
  {
    bCustomLayoutEnabled = v.toBool(false);
  }
  v = p["sCustomLayout"];
  QString sCustomLayout = QString();
  if (!v.isUndefined())
  {
    sCustomLayout = v.toString(QString());
  }
  SlotCustomTransitionChanged(bCustomLayoutEnabled, sCustomLayout);
  m_bIsInUndoOperation = false;
}

//----------------------------------------------------------------------------------------
//
void CUndoPathSplitterModelWithWidget::OnUndoStackSet()
{
  /*
  if (nullptr != m_pWidget)
  {
    m_pWidget->SetUndoStack(m_pUndoStack);
  }
  */
}
