#ifndef NODEDEBUGWIDGET_H
#define NODEDEBUGWIDGET_H

#include "Systems/Nodes/NodeModelBase.h"

#include <nodes/NodeGeometry>
#include <nodes/NodeState>
#include <nodes/internal/NodeGraphicsObject.hpp>

#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QPointer>
#include <QWidget>

#include <memory>
#include <variant>

class CNodeDebugWidget;
class CNodeEditorFlowScene;
class CNodeEditorFlowView;
class CSceneNodeResolver;
namespace Ui {
  class CNodeDebugWidget;
}
struct SScene;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SScene> tspScene;


class CNodeContainingItem
{
public:
  CNodeContainingItem(QtNodes::Node* pNode);
  virtual ~CNodeContainingItem();

  QtNodes::Node* Node() const;

protected:
  QtNodes::Node*        m_pNode;
};

//----------------------------------------------------------------------------------------
//
class CNodeMock : public QFrame, public CNodeContainingItem
{
  Q_OBJECT

public:
  explicit CNodeMock(std::unique_ptr<QtNodes::NodeDataModel>&& dataModel,
                     QtNodes::Node* pNode,
                     QWidget* pParent = nullptr, QWidget* pView = nullptr);
  ~CNodeMock();

  void Resize(QSize s);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void mouseDoubleClickEvent(QMouseEvent* pEvt) override;
  void paintEvent(QPaintEvent* pEvt) override;
  QSize sizeHint() const override;

  std::unique_ptr<QtNodes::NodeDataModel> m_spDataModel;
  QPointer<QWidget>     m_pActualParent;
  QtNodes::NodeGeometry m_geometry;
  QtNodes::NodeState    m_state;
  qint32                m_iConnectionPointDiameter = 0;
};

//----------------------------------------------------------------------------------------
//
class CNodeDebugNodeStartEnd : public CNodeMock
{
  Q_OBJECT

public:
  explicit CNodeDebugNodeStartEnd(bool bStart, QtNodes::Node* pNode,
                                  QWidget* pParent = nullptr, QWidget* pView = nullptr);
  ~CNodeDebugNodeStartEnd();

  void paintEvent(QPaintEvent* pEvt) override;
  QSize sizeHint() const override;

private:
  QtNodes::NodeGeometry m_geometryFixed;
};

//----------------------------------------------------------------------------------------
//
class CNodeDebugNode : public CNodeMock
{
  Q_OBJECT

public:
  explicit CNodeDebugNode(const tspProject& spProject, const QString& sScene,
                          QtNodes::Node* pNode,
                          QWidget* pParent = nullptr, QWidget* pView = nullptr);
  ~CNodeDebugNode();

  void paintEvent(QPaintEvent* pEvt) override;
};

//----------------------------------------------------------------------------------------
//
class CNodeDebugError : public QFrame
{
  Q_OBJECT

public:
  explicit CNodeDebugError(QString sError, QtMsgType type, QWidget* pParent = nullptr,
                           QWidget* pView = nullptr);
  ~CNodeDebugError();

  void PushError(const QString& sError);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void paintEvent(QPaintEvent* pEvt) override;
  QSize sizeHint() const override;

protected:
  QPointer<QLabel>      m_pLabel;
  QColor                m_errorColor;
  QColor                m_normalBoundaryColor;
  qint32                m_iPenWidth = 0;
  qint32                m_iConnectionPointDiameter = 0;
};

//----------------------------------------------------------------------------------------
//
class CNodeDebugSelection : public CNodeMock
{
  Q_OBJECT

public:
  explicit CNodeDebugSelection(const QStringList& vsScenes,
                               QtNodes::Node* pNode,
                               QWidget* pParent = nullptr,
                               CNodeDebugWidget* pView = nullptr);
  ~CNodeDebugSelection();

  QString CurrentScene() const;
  qint32 CurrentIndex() const;

  void paintEvent(QPaintEvent* pEvt) override;

protected:
  QPointer<QComboBox> m_pComboSelection;
  QStringList m_vsScenes;
};

//----------------------------------------------------------------------------------------
//
class CNodeDebugBackground : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QColor backgroundColor READ BackgroundColor WRITE SetBackgroundColor)

public:
  explicit CNodeDebugBackground(QWidget* pParent = nullptr);
  ~CNodeDebugBackground();

  void SetBackgroundColor(const QColor& col);
  QColor BackgroundColor() const { return m_backgroundColor; }

protected:
  void paintEvent(QPaintEvent* pEvt) override;

private:
  QColor m_backgroundColor;
};

//----------------------------------------------------------------------------------------
//
class CNodeDebugWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QColor backgroundColor READ BackgroundColor WRITE SetBackgroundColor)

public:
  explicit CNodeDebugWidget(QWidget* pParent = nullptr);
  ~CNodeDebugWidget();

  void Initialize(QPointer<CNodeEditorFlowView> pFlowView,
                  QPointer<CNodeEditorFlowScene> pScene);
  void NextScene(qint32 iIndex = -1);
  void StartDebug(const tspProject& spProject, const std::variant<QString, QUuid>& start);
  void StopDebug();

  QColor BackgroundColor() const;
  void SetBackgroundColor(const QColor& col);

  void FocusNode(QtNodes::Node* pNode);

private slots:
  void SlotSceneError(QString sError, QtMsgType type);
  void SlotUpdateScene();
  void SlotUpdateScrollAndScene();

private:
  void AddWidget(QWidget* pWidget);
  void Clear();
  bool IsInErrorState() const;
  QWidget* LastWidget() const;
  QtNodes::Node* NodeFromScene(QtNodes::Node* pLocalNode);
  void ScrollToEnd();
  void UpdateNodeContainingItemNode(CNodeContainingItem* pItem,
                                    CNodeModelBase::EDebugState state);

  std::unique_ptr<Ui::CNodeDebugWidget> m_spUi;
  std::unique_ptr<CSceneNodeResolver>   m_spNodeResolver;
  tspProject                            m_spCurrentProject;
  QPointer<CNodeEditorFlowView>         m_pFlowView;
  QPointer<CNodeEditorFlowScene>        m_pScene;
};

#endif // NODEDEBUGWIDGET_H
