#include "ButtplugDeviceConnector.h"
#include "Application.h"
#include "ButtplugDeviceConnectorContext.h"
#include "ButtplugDeviceWrapper.h"
#include "DeviceSettings.h"

#include "Systems/HelpFactory.h"

#include "Widgets/HelpOverlay.h"

#include "Utils/WidgetHelpers.h"

#include <ButtplugClient.h>
#include <ButtplugDevice.h>

#include <QDebug>
#include <QDirIterator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLibraryInfo>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpacerItem>
#include <QSpinBox>
#include <QWidget>
#include <QtGlobal>

#include <future>
#include <limits>

namespace
{
  const char c_sInitFacePortSettingName[] = "initfacePort";
  const char c_sInitFaceLocationSettingName[] = "initfaceInstall";

  const char c_sInitfacePluginFolder[] = "/buttplug";

  const QString c_sInitfacePortSettingHelpId = "Settings/InitfacePort";
  const QString c_sInitfaceInstallSettingHelpId = "Settings/InitfaceInstallLocation";

  //--------------------------------------------------------------------------------------
  //
  QWidget* CreatePortWidget(QWidget* pParent)
  {
    auto pSetting = CDeviceSettingFactory::Setting<quint16>(c_sInitFacePortSettingName);

    QWidget* pRoot = new QWidget(pParent);
    pRoot->setObjectName("initfacePortWidget");
    QHBoxLayout* pLayout = new QHBoxLayout(pRoot);
    pLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* pLabel = new QLabel("Initface port", pRoot);
    pLayout->addWidget(pLabel);
    pLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    QSpinBox* pSpinBox = new QSpinBox(pRoot);
    pSpinBox->setRange(0, std::numeric_limits<quint16>::max());
    pSpinBox->setValue(pSetting->GetValue());
    QObject::connect(pSpinBox, qOverload<qint32>(&QSpinBox::valueChanged), pSpinBox,
                     [](qint32 iValue) {
      auto pSetting = CDeviceSettingFactory::Setting<quint16>(c_sInitFacePortSettingName);
      pSetting->Store(static_cast<quint16>(iValue));
    });
    pLayout->addWidget(pSpinBox);

    auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
    if (nullptr != wpHelpFactory)
    {
      pRoot->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sInitfacePortSettingHelpId);
      wpHelpFactory->RegisterHelp(c_sInitfacePortSettingHelpId, ":/devices/resources/help/settings/initface_port_setting_help.html");
    }

    return pRoot;
  }

  //--------------------------------------------------------------------------------------
  //
  QWidget* CreateInstallLocaionWidget(QWidget* pParent)
  {
    auto pSetting = CDeviceSettingFactory::Setting<QString>(c_sInitFaceLocationSettingName);

    QWidget* pRoot = new QWidget(pParent);
    pRoot->setObjectName("initfaceInstallLocationWidget");
    QHBoxLayout* pLayout = new QHBoxLayout(pRoot);
    pLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* pLabel = new QLabel("Initface Central install location", pRoot);
    pLayout->addWidget(pLabel);
    pLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    QLineEdit* pLineEdit = new QLineEdit(pRoot);
    pLineEdit->setText(pSetting->GetValue());
    QObject::connect(pLineEdit, &QLineEdit::editingFinished, pLineEdit,
                     [pLineEdit]() {
      auto pSetting = CDeviceSettingFactory::Setting<QString>(c_sInitFaceLocationSettingName);
      pSetting->Store(pLineEdit->text());
    });
    pLayout->addWidget(pLineEdit);
    QPushButton* pBrowseButton = new QPushButton("...", pRoot);
    pBrowseButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    pBrowseButton->setObjectName("BrowseButton");
    QObject::connect(pBrowseButton, &QPushButton::clicked, pBrowseButton,
                     [pLineEdit, pBrowseButton]() {
      auto pSetting = CDeviceSettingFactory::Setting<QString>(c_sInitFaceLocationSettingName);

      QPointer<QPushButton> pGuard(pBrowseButton);
      QString sPath =
          widget_helpers::GetExistingFile(pGuard, QObject::tr("Select Initface Central"),
            pSetting->GetValue(),
            QFileDialog::DontResolveSymlinks);
      if (nullptr == pGuard) { return; }

      pSetting->Store(sPath);
      QSignalBlocker blocker(pLineEdit);
      pLineEdit->setText(sPath);
    });
    pLayout->addWidget(pBrowseButton);

    auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
    if (nullptr != wpHelpFactory)
    {
      pRoot->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sInitfaceInstallSettingHelpId);
      wpHelpFactory->RegisterHelp(c_sInitfaceInstallSettingHelpId, ":/devices/resources/help/settings/initface_install_setting_help.html");
    }

    return pRoot;
  }
}

// include here because of max bullcrap
#if defined(Q_OS_WIN)
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#endif

namespace
{
  //--------------------------------------------------------------------------------------
  //
#if defined(Q_OS_WIN)
  bool MatchProcessName(DWORD processID, std::string processName)
  {
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.
    if (NULL != hProcess)
    {
      HMODULE hMod;
      DWORD cbNeeded;

      if ( EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
      {
          GetModuleBaseName( hProcess, hMod, szProcessName,
                             sizeof(szProcessName)/sizeof(TCHAR) );
      }
    }

    // Compare process name with your string
    QString sProcessNme = QString::fromStdString(processName);
    QString sFoundProcessName =  QString::fromWCharArray(szProcessName);
    bool matchFound = sFoundProcessName.contains(sProcessNme);

    // Release the handle to the process.
    CloseHandle(hProcess);

    return matchFound;
  }
#endif

  qint32 FindProcessId(QString sProcess)
  {
    qint32 iRet = -1;

#if defined(Q_OS_WIN)
    // Get the list of process identifiers.
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;
    if ( !EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return 1;
    }

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.
    for (i = 0; i < cProcesses; i++)
    {
      if(aProcesses[i] != 0)
      {
        if (MatchProcessName(aProcesses[i], sProcess.toStdString()))
        {
          iRet = aProcesses[i];
          break;
        }
      }
    }
#endif

    return iRet;
  }
}

//--------------------------------------------------------------------------------------
//
class CInitfaceEngineClientWrapper
{
public:
  CInitfaceEngineClientWrapper(const QString& sLoadPath,
                               std::function<void(const QString&)> fnDeviceAdded,
                               std::function<void(const QString&)> fnDeviceRemoved,
                               std::function<void()> fnDisconnected);
  ~CInitfaceEngineClientWrapper();

  bool Connect();
  qint32 DeviceCount() const;
  const std::map<QString, std::weak_ptr<Buttplug::Device>>& Devices() const;
  bool Disconnect();
  bool IsConnected();
  bool IsLoaded() const { return m_bIsLoaded; };
  bool StartScanning();
  bool StopScanning();

private:
  std::unique_ptr<Buttplug::Client>                  m_spClient;
  std::map<QString, std::weak_ptr<Buttplug::Device>> m_deviceList;
  bool                                               m_bIsLoaded = false;
};

//----------------------------------------------------------------------------------------
//
CButtplugDeviceConnector::CButtplugDeviceConnector() :
  QObject(),
  IDeviceConnector(),
  m_spClient(nullptr),
  m_pContext(new CButtplugDeviceConnectorContext(this))
{
  m_spClient.reset(new CInitfaceEngineClientWrapper(
                     FindPluginPath(),
                     [this](const QString& sDevice) {
                       emit SignalDeviceCountChanged();
                     },
                     [this](const QString& sDevice) {
                       emit m_pContext->SignalDeviceRemoved(sDevice);
                       emit SignalDeviceCountChanged();
                     },
                     [this]() {
                       emit SignalDisconnected();
                     }));
}
CButtplugDeviceConnector::~CButtplugDeviceConnector()
{
}

//----------------------------------------------------------------------------------------
//
QStringList CButtplugDeviceConnector::DeviceNames() const
{
  if (!m_spClient->IsLoaded())
  {
    return {};
  }

  QStringList vsRet;
  for (const auto& [sName, _] : m_spClient->Devices())
  {
    vsRet.push_back(sName);
  }
  return vsRet;
}

//----------------------------------------------------------------------------------------
//
std::vector<std::shared_ptr<IDevice>> CButtplugDeviceConnector::Devices() const
{
  if (!m_spClient->IsLoaded())
  {
    return {};
  }

  std::vector<std::shared_ptr<IDevice>> ret;
  for (const auto& [_, wpDevice] : m_spClient->Devices())
  {
    ret.push_back(std::make_shared<CButtplugDeviceWrapper>(wpDevice, m_pContext));
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceConnector::StartScanning()
{
  if (m_spClient->IsLoaded())
  {
    m_spClient->StartScanning();
  }
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceConnector::StopScanning()
{
  if (m_spClient->IsLoaded())
  {
    m_spClient->StopScanning();
  }
}

//----------------------------------------------------------------------------------------
//
QString CButtplugDeviceConnector::FindPluginPath() const
{
  QString sPluginPath = QLibraryInfo::location(QLibraryInfo::PluginsPath);
  return sPluginPath + c_sInitfacePluginFolder;
}

//----------------------------------------------------------------------------------------
//
CInitfaceEngineClientWrapper::CInitfaceEngineClientWrapper(
    const QString& sLoadPath,
    std::function<void(const QString&)> fnDeviceAdded,
    std::function<void(const QString&)> fnDeviceRemoved,
    std::function<void()> fnDisconnected)
{
  if(!Buttplug::FFI::Init(sLoadPath.toStdString()))
  {
    qWarning() << "Failed to load buttplug_rs_ffi functions.";
    return;
  }

  Buttplug::FFI::ActivateEnvLogger();

  m_bIsLoaded = true;
  m_spClient.reset(new Buttplug::Client("JoipEngineClient"));

  m_spClient->DeviceAddedCb = [this, fnDeviceAdded](std::weak_ptr<Buttplug::Device> device)
  {
    if (auto spDevice = device.lock(); nullptr != spDevice)
    {
      qDebug() << "Initface Client: Device added";
      const QString sName = QString::fromStdString(spDevice->Name());
      m_deviceList.insert({sName, device});
      if (nullptr != fnDeviceAdded)
      {
        fnDeviceAdded(sName);
      }
    }
  };

  m_spClient->DeviceRemovedCb = [this, fnDeviceRemoved](std::weak_ptr<Buttplug::Device> device)
  {
    if (auto spDevice = device.lock(); nullptr != spDevice)
    {
      qDebug() << "Initface Client: Device removed";
      const QString sName = QString::fromStdString(spDevice->Name());
      auto it = m_deviceList.find(sName);
      if (m_deviceList.end() != it)
      {
        m_deviceList.erase(it);
        if (nullptr != fnDeviceRemoved)
        {
          fnDeviceRemoved(sName);
        }
      }
    }
  };

  m_spClient->ErrorReceivedCb = [](const std::string& error)
  {
    qWarning() << QString("Initface Client error: %1").arg(QString::fromStdString(error));
  };

  m_spClient->ScanningFinishedCb = []()
  {
    qDebug() << "Initface Client: Scanning finished";
  };

  m_spClient->PingTimeoutCb = []()
  {
    qWarning() << "Initface Client: Ping timeout";
  };

  m_spClient->ServerDisconnectCb = [fnDisconnected]()
  {
    qDebug() << "Initface Client Server disconnect";
    if (nullptr != fnDisconnected)
    {
      fnDisconnected();
    }
  };
}
CInitfaceEngineClientWrapper::~CInitfaceEngineClientWrapper()
{
}

//----------------------------------------------------------------------------------------
//
bool CInitfaceEngineClientWrapper::Connect()
{
  if (nullptr == m_spClient) { return false; }
  auto pSetting = CDeviceSettingFactory::Setting<quint16>(c_sInitFacePortSettingName);
  if (nullptr == pSetting)
  {
    return false;
  }

  return m_spClient->ConnectWebsocket(
        QString("ws://127.0.0.1:%1/buttplug")
        .arg(pSetting->GetValue()).toStdString(), true).get();
}

//----------------------------------------------------------------------------------------
//
qint32 CInitfaceEngineClientWrapper::DeviceCount() const
{
  if (nullptr == m_spClient) { return false; }
  return m_spClient->DeviceCount();
}

//----------------------------------------------------------------------------------------
//
const std::map<QString, std::weak_ptr<Buttplug::Device>>&
CInitfaceEngineClientWrapper::Devices() const
{
  return m_deviceList;
}

//----------------------------------------------------------------------------------------
//
bool CInitfaceEngineClientWrapper::Disconnect()
{
  if (nullptr == m_spClient) { return false; }
  return m_spClient->Disconnect().get();
}

//----------------------------------------------------------------------------------------
//
bool CInitfaceEngineClientWrapper::IsConnected()
{
  return nullptr == m_spClient && m_spClient->Connected();
}

//----------------------------------------------------------------------------------------
//
bool CInitfaceEngineClientWrapper::StartScanning()
{
  if (nullptr == m_spClient)
  {
    return false;
  }

  return m_spClient->StartScanning().get();
}

//----------------------------------------------------------------------------------------
//
bool CInitfaceEngineClientWrapper::StopScanning()
{
  if (nullptr == m_spClient)
  {
    return false;
  }

  return m_spClient->StopScanning().get();
}

//----------------------------------------------------------------------------------------
//
CInitfaceEngineDeviceConnector::CInitfaceEngineDeviceConnector() :
  CButtplugDeviceConnector()
{
}
CInitfaceEngineDeviceConnector::~CInitfaceEngineDeviceConnector()
{
}

//----------------------------------------------------------------------------------------
//
bool CInitfaceEngineDeviceConnector::Connect()
{
  if (!m_spClient->IsLoaded()) { return false; }
  if (StartEngine())
  {
    // we need to wait for the server to start, sadly we can't synchronize this better
    QThread::sleep(15);

    if (!m_spClient->Connect())
    {
      m_pInitfaceEngineProcess->terminate();
      delete m_pInitfaceEngineProcess;
    }
  }
  return false;
}

//----------------------------------------------------------------------------------------
//
void CInitfaceEngineDeviceConnector::Disconnect()
{
  if (m_spClient->IsLoaded())
  {
    m_spClient->Disconnect();
  }

  m_pInitfaceEngineProcess->terminate();
  delete m_pInitfaceEngineProcess;
}

//----------------------------------------------------------------------------------------
//
void CInitfaceEngineDeviceConnector::SlotProcessError(QProcess::ProcessError error)
{
  switch(error)
  {
    case QProcess::ProcessError::ReadError: break;
    case QProcess::ProcessError::WriteError: break;
    default:
      emit SignalDisconnected();
      break;
  }
}

//----------------------------------------------------------------------------------------
//
void CInitfaceEngineDeviceConnector::SlotReadyReadStdOut()
{
#ifndef NDEBUG
  qDebug() << "Initface Engine:" << m_pInitfaceEngineProcess->readAllStandardOutput();
#endif
}

//----------------------------------------------------------------------------------------
//
bool CInitfaceEngineDeviceConnector::StartEngine()
{
  m_pInitfaceEngineProcess = new QProcess(this);
  connect(m_pInitfaceEngineProcess, &QProcess::errorOccurred, this,
          &CInitfaceEngineDeviceConnector::SlotProcessError);
  connect(m_pInitfaceEngineProcess, &QProcess::readyReadStandardOutput, this,
          &CInitfaceEngineDeviceConnector::SlotReadyReadStdOut);
  connect(m_pInitfaceEngineProcess, &QProcess::readyReadStandardError, this,
          &CInitfaceEngineDeviceConnector::SlotReadyReadStdOut);

  const QString sPluginPath = FindPluginPath();
  QString sExecutable;
  QDirIterator iter(sPluginPath, QStringList() << "intiface-engine*",
                    QDir::Files | QDir::NoDotAndDotDot | QDir::Executable,
                    QDirIterator::NoIteratorFlags);
  if (iter.hasNext())
  {
    sExecutable = iter.next();
  }
  if (sExecutable.isEmpty())
  {
    qWarning() << "Could not locate the initface-engine executable.";
    delete m_pInitfaceEngineProcess;
    return false;
  }

  auto pSetting = CDeviceSettingFactory::Setting<quint16>(c_sInitFacePortSettingName);
  if (nullptr == pSetting)
  {
    qWarning() << "Could not load initface port setting.";
    delete m_pInitfaceEngineProcess;
    return false;
  }

  QStringList args = QStringList() <<
                     QString("--websocket-port %1").arg(pSetting->GetValue()) <<
                     "--use-bluetooth-le" << "--use-serial" << "--use-hid" <<
                     "--use-lovense-dongle-serial" << "--use-lovense-dongle-hid" <<
                     "--use-xinput" << "--use-lovense-connect";

  m_pInitfaceEngineProcess->start(sExecutable + " " + args.join(" "));
  m_pInitfaceEngineProcess->waitForStarted();

  if (QProcess::NotRunning != m_pInitfaceEngineProcess->state())
  {
    return true;
  }

  delete m_pInitfaceEngineProcess;
  return false;
}

//----------------------------------------------------------------------------------------
//
CInitfaceCentralDeviceConnector::CInitfaceCentralDeviceConnector() :
  CButtplugDeviceConnector()
{

}
CInitfaceCentralDeviceConnector::~CInitfaceCentralDeviceConnector()
{

}

//----------------------------------------------------------------------------------------
//
bool CInitfaceCentralDeviceConnector::Connect()
{
  if (!m_spClient->IsLoaded()) { return false; }

  if (!FindInitfaceProcess())
  {
    if (StartInitface())
    {
      // we return true, because the engine is started but sadly we can't currently start
      // it with the server already running
      return true;
    }
  }
  else
  {
    return m_spClient->Connect();
  }

  return false;
}

//----------------------------------------------------------------------------------------
//
void CInitfaceCentralDeviceConnector::Disconnect()
{
  if (m_spClient->IsLoaded())
  {
    m_spClient->Disconnect();
  }
}

//----------------------------------------------------------------------------------------
//
void CInitfaceCentralDeviceConnector::SlotProcessError(QProcess::ProcessError error)
{
  switch(error)
  {
    case QProcess::ProcessError::ReadError: break;
    case QProcess::ProcessError::WriteError: break;
    default:
      emit SignalDisconnected();
      break;
  }
}

//----------------------------------------------------------------------------------------
//
bool CInitfaceCentralDeviceConnector::FindInitfaceProcess()
{
  qint32 iPid = FindProcessId("initface_central");
  if (-1 == iPid)
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CInitfaceCentralDeviceConnector::StartInitface()
{
  const QString sPluginPath = FindPluginPath();

  auto pSetting = CDeviceSettingFactory::Setting<QString>(c_sInitFaceLocationSettingName);
  if (nullptr == pSetting)
  {
    qWarning() << "Could not load initface install location setting.";
    return false;
  }

  QString sPath = pSetting->GetValue();
  if (sPath.isEmpty())
  {
    return false;
  }

  return QProcess::startDetached(pSetting->GetValue());
}

//----------------------------------------------------------------------------------------
// registering settings here
DECLARE_DEVICE_SETTING(struct SInitfacePort, c_sInitFacePortSettingName, quint16, 12345,
                       ::CreatePortWidget)
DECLARE_DEVICE_SETTING(struct SInitfaceInstallLocation, c_sInitFaceLocationSettingName, QString, QString(),
                       ::CreateInstallLocaionWidget)
