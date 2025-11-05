#ifndef CEDITORDEBUGGABLEWIDGET
#define CEDITORDEBUGGABLEWIDGET

#include "EditorWidgetBase.h"

#include "Systems/Database/Scene.h"

#include <QPointer>
#include <QPushButton>

#include <functional>
#include <variant>

class CScriptRunner;

class CEditorDebuggableWidget : public CEditorWidgetBase
{
  Q_OBJECT

public:
  CEditorDebuggableWidget(QWidget* pParent = nullptr);
  ~CEditorDebuggableWidget() override;

  using tSceneToDebug = std::variant<QString,tspScene,std::nullptr_t>;
  using tFnGetScene = std::function<tSceneToDebug()>;
  void Initalize(QPointer<QWidget> pSceneView,
                 tFnGetScene fnGetScene);
  void UpdateButtons(QPointer<QPushButton> pDebugButton,
                     QPointer<QPushButton> pStopDebugButton);

  void UnloadProject() override final;
  virtual void UnloadProjectImpl() = 0;

signals:
  void SignalDebugFinished();
  void SignalDebugStarted();
  void SignalExecutionError(QString sException, qint32 iLine, QString sStack);

protected slots:
  void SlotDebugStart();
  void SlotDebugStop();
  void SlotDebugUnloadFinished();

protected:
  tspProject                                             m_spCurrentProject;

private:
  std::weak_ptr<CScriptRunner>                           m_wpScriptRunner;
  QPointer<QWidget>                                      m_pSceneView;
  QPointer<QPushButton>                                  m_pDebugButton;
  QPointer<QPushButton>                                  m_pStopDebugButton;
  tFnGetScene                                            m_fnGetScene;
  QMetaObject::Connection                                m_debugFinishedConnection;
  bool                                                   m_bDebugging;
};

#endif // CEDITORDEBUGGABLEWIDGET
