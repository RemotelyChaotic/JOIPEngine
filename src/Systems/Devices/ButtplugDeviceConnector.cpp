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
  const char c_sIntifacePortSettingName[] = "intifacePort";
  const char c_sIntifaceLocationSettingName[] = "intifaceInstall";

  const char c_sIntifacePluginFolder[] = "/buttplug";

  const QString c_sIntifacePortSettingHelpId = "Settings/IntifacePort";
  const QString c_sIntifaceInstallSettingHelpId = "Settings/IntifaceInstallLocation";

  //--------------------------------------------------------------------------------------
  //
  QWidget* CreatePortWidget(QWidget* pParent)
  {
    auto pSetting = CDeviceSettingFactory::Setting<quint16>(c_sIntifacePortSettingName);

    QWidget* pRoot = new QWidget(pParent);
    pRoot->setObjectName("intifacePortWidget");
    QHBoxLayout* pLayout = new QHBoxLayout(pRoot);
    pLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* pLabel = new QLabel("Intiface port", pRoot);
    pLayout->addWidget(pLabel);
    pLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    QSpinBox* pSpinBox = new QSpinBox(pRoot);
    pSpinBox->setRange(0, std::numeric_limits<quint16>::max());
    pSpinBox->setValue(pSetting->GetValue());
    QObject::connect(pSpinBox, qOverload<qint32>(&QSpinBox::valueChanged), pSpinBox,
                     [](qint32 iValue) {
      auto pSetting = CDeviceSettingFactory::Setting<quint16>(c_sIntifacePortSettingName);
      pSetting->Store(static_cast<quint16>(iValue));
    });
    pLayout->addWidget(pSpinBox);

    auto wpHelpFactory = CApplication::Instance()->System<CHelpFactory>().lock();
    if (nullptr != wpHelpFactory)
    {
      pRoot->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sIntifacePortSettingHelpId);
      wpHelpFactory->RegisterHelp(c_sIntifacePortSettingHelpId, ":/devices/resources/help/settings/intiface_port_setting_help.html");
    }

    return pRoot;
  }

  //--------------------------------------------------------------------------------------
  //
  QWidget* CreateInstallLocaionWidget(QWidget* pParent)
  {
    auto pSetting = CDeviceSettingFactory::Setting<QString>(c_sIntifaceLocationSettingName);

    QWidget* pRoot = new QWidget(pParent);
    pRoot->setObjectName("intifaceInstallLocationWidget");
    QHBoxLayout* pLayout = new QHBoxLayout(pRoot);
    pLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* pLabel = new QLabel("Intiface Central install location", pRoot);
    pLayout->addWidget(pLabel);
    pLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    QLineEdit* pLineEdit = new QLineEdit(pRoot);
    pLineEdit->setText(pSetting->GetValue());
    QObject::connect(pLineEdit, &QLineEdit::editingFinished, pLineEdit,
                     [pLineEdit]() {
      auto pSetting = CDeviceSettingFactory::Setting<QString>(c_sIntifaceLocationSettingName);
      pSetting->Store(pLineEdit->text());
    });
    pLayout->addWidget(pLineEdit);
    QPushButton* pBrowseButton = new QPushButton("...", pRoot);
    pBrowseButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    pBrowseButton->setObjectName("BrowseButton");
    QObject::connect(pBrowseButton, &QPushButton::clicked, pBrowseButton,
                     [pLineEdit, pBrowseButton]() {
      auto pSetting = CDeviceSettingFactory::Setting<QString>(c_sIntifaceLocationSettingName);

      QPointer<QPushButton> pGuard(pBrowseButton);
      QString sPath =
          widget_helpers::GetExistingFile(pGuard, QObject::tr("Select Intiface Central"),
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
      pRoot->setProperty(helpOverlay::c_sHelpPagePropertyName, c_sIntifaceInstallSettingHelpId);
      wpHelpFactory->RegisterHelp(c_sIntifaceInstallSettingHelpId, ":/devices/resources/help/settings/intiface_install_setting_help.html");
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

    //qDebug() << sFoundProcessName << matchFound;

    // Release the handle to the process.
    CloseHandle(hProcess);

    return matchFound;
  }
#endif

  //--------------------------------------------------------------------------------------
  //
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
class CIntifaceEngineClientWrapper
{
public:
  CIntifaceEngineClientWrapper(const QString& sLoadPath,
                               std::function<void(const QString&)> fnDeviceAdded,
                               std::function<void(const QString&)> fnDeviceRemoved,
                               std::function<void()> fnDisconnected);
  ~CIntifaceEngineClientWrapper();

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
  Buttplug::DeviceAddedCallback                      m_fnDeviceAddedCb;
  Buttplug::DeviceRemovedCallback                    m_fnDeviceRemovedCb;
  Buttplug::ErrorReceivedCallback                    m_fnErrorReceivedCb;
  Buttplug::ScanningFinishedCallback                 m_fnScanningFinishedCb;
  Buttplug::PingTimeoutCallback                      m_fnPingTimeoutCb;
  Buttplug::ServerDisconnectCallback                 m_fnServerDisconnectCb;
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
  m_spClient.reset(new CIntifaceEngineClientWrapper(
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
std::shared_ptr<IDevice> CButtplugDeviceConnector::Device(const QString& sName) const
{
  if (!m_spClient->IsLoaded())
  {
    return nullptr;
  }

  auto it = m_spClient->Devices().find(sName);
  if (m_spClient->Devices().end() != it)
  {
    return std::make_shared<CButtplugDeviceWrapper>(it->second, m_pContext);
  }
  return nullptr;
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
bool CButtplugDeviceConnector::IsConnected() const
{
  if (!m_spClient->IsLoaded())
  {
    return false;
  }

  return m_spClient->IsConnected();
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
  return sPluginPath + c_sIntifacePluginFolder;
}

//----------------------------------------------------------------------------------------
//
CIntifaceEngineClientWrapper::CIntifaceEngineClientWrapper(
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

  m_fnDeviceAddedCb = [this, fnDeviceAdded](std::weak_ptr<Buttplug::Device> device)
  {
    if (auto spDevice = device.lock(); nullptr != spDevice)
    {
      qDebug() << "Intiface Client: Device added";
      const QString sName = QString::fromStdString(spDevice->Name());
      m_deviceList.insert({sName, device});
      if (nullptr != fnDeviceAdded)
      {
        fnDeviceAdded(sName);
      }
    }
  };

  m_fnDeviceRemovedCb = [this, fnDeviceRemoved](std::weak_ptr<Buttplug::Device> device)
  {
    if (auto spDevice = device.lock(); nullptr != spDevice)
    {
      qDebug() << "Intiface Client: Device removed";
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

  m_fnErrorReceivedCb = [](const std::string& error)
  {
    qWarning() << QString("Intiface Client error: %1").arg(QString::fromStdString(error));
  };

  m_fnScanningFinishedCb = []()
  {
    qDebug() << "Intiface Client: Scanning finished";
  };

  m_fnPingTimeoutCb = []()
  {
    qWarning() << "Intiface Client: Ping timeout";
  };

  m_fnServerDisconnectCb = [fnDisconnected]()
  {
    qDebug() << "Intiface Client Server disconnect";
    if (nullptr != fnDisconnected)
    {
      fnDisconnected();
    }
  };
}
CIntifaceEngineClientWrapper::~CIntifaceEngineClientWrapper()
{
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineClientWrapper::Connect()
{
  if (!m_bIsLoaded) { return false; }

  m_spClient.reset(new Buttplug::Client("JOIPEngine"));

  m_spClient->DeviceAddedCb = m_fnDeviceAddedCb;
  m_spClient->DeviceRemovedCb = m_fnDeviceRemovedCb;
  m_spClient->ErrorReceivedCb = m_fnErrorReceivedCb;
  m_spClient->ScanningFinishedCb = m_fnScanningFinishedCb;
  m_spClient->PingTimeoutCb = m_fnPingTimeoutCb;
  m_spClient->ServerDisconnectCb = m_fnServerDisconnectCb;

  auto pSetting = CDeviceSettingFactory::Setting<quint16>(c_sIntifacePortSettingName);
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
qint32 CIntifaceEngineClientWrapper::DeviceCount() const
{
  if (nullptr == m_spClient) { return false; }
  return m_spClient->DeviceCount();
}

//----------------------------------------------------------------------------------------
//
const std::map<QString, std::weak_ptr<Buttplug::Device>>&
CIntifaceEngineClientWrapper::Devices() const
{
  return m_deviceList;
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineClientWrapper::Disconnect()
{
  if (nullptr == m_spClient) { return false; }

  while (!m_deviceList.empty())
  {
    (*m_spClient->DeviceRemovedCb)(m_deviceList.begin()->second);
  }

  bool bOk = m_spClient->Disconnect().get();

  m_spClient.reset(nullptr);

  return bOk;
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineClientWrapper::IsConnected()
{
  return nullptr != m_spClient && m_spClient->Connected();
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineClientWrapper::StartScanning()
{
  if (nullptr == m_spClient)
  {
    return false;
  }

  return m_spClient->StartScanning().get();
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineClientWrapper::StopScanning()
{
  if (nullptr == m_spClient)
  {
    return false;
  }

  return m_spClient->StopScanning().get();
}

//----------------------------------------------------------------------------------------
//
CIntifaceEngineDeviceConnector::CIntifaceEngineDeviceConnector() :
  CButtplugDeviceConnector()
{
}
CIntifaceEngineDeviceConnector::~CIntifaceEngineDeviceConnector()
{
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineDeviceConnector::CanConnect()
{
  if (!m_spClient->IsLoaded()) { return false; }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineDeviceConnector::Connect()
{
  if (!m_spClient->IsLoaded()) { return false; }
  if (!StartEngine())
  {
    return false;
  }
  // we need to wait for the server to start, sadly we can't synchronize this better
  QThread::sleep(15);
  if (!m_spClient->Connect())
  {
    if (nullptr != m_pIntifaceEngineProcess)
    {
      m_pIntifaceEngineProcess->terminate();
      delete m_pIntifaceEngineProcess;
    }
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
void CIntifaceEngineDeviceConnector::Disconnect()
{
  if (m_spClient->IsLoaded())
  {
    m_spClient->Disconnect();
  }

  if (nullptr != m_pIntifaceEngineProcess)
  {
    m_pIntifaceEngineProcess->terminate();
    delete m_pIntifaceEngineProcess;
  }
}

//----------------------------------------------------------------------------------------
//
void CIntifaceEngineDeviceConnector::SlotProcessError(QProcess::ProcessError error)
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
void CIntifaceEngineDeviceConnector::SlotReadyReadStdOut()
{
#ifndef NDEBUG
  qDebug() << "Intiface Engine:" << m_pIntifaceEngineProcess->readAllStandardOutput();
#endif
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineDeviceConnector::StartEngine()
{
  m_pIntifaceEngineProcess = new QProcess(this);
  connect(m_pIntifaceEngineProcess, &QProcess::errorOccurred, this,
          &CIntifaceEngineDeviceConnector::SlotProcessError);
  connect(m_pIntifaceEngineProcess, &QProcess::readyReadStandardOutput, this,
          &CIntifaceEngineDeviceConnector::SlotReadyReadStdOut);
  connect(m_pIntifaceEngineProcess, &QProcess::readyReadStandardError, this,
          &CIntifaceEngineDeviceConnector::SlotReadyReadStdOut);

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
    qWarning() << "Could not locate the intiface-engine executable.";
    delete m_pIntifaceEngineProcess;
    return false;
  }

  auto pSetting = CDeviceSettingFactory::Setting<quint16>(c_sIntifacePortSettingName);
  if (nullptr == pSetting)
  {
    qWarning() << "Could not load intiface port setting.";
    delete m_pIntifaceEngineProcess;
    return false;
  }

  QStringList args = QStringList() <<
                     QString("--websocket-port %1").arg(pSetting->GetValue()) <<
                     "--use-bluetooth-le" << "--use-serial" << "--use-hid" <<
                     "--use-lovense-dongle-serial" << "--use-lovense-dongle-hid" <<
                     "--use-xinput" << "--use-lovense-connect";

  m_pIntifaceEngineProcess->start(sExecutable + " " + args.join(" "));
  m_pIntifaceEngineProcess->waitForStarted();

  if (QProcess::NotRunning != m_pIntifaceEngineProcess->state())
  {
    return true;
  }

  delete m_pIntifaceEngineProcess;
  return false;
}

//----------------------------------------------------------------------------------------
//
CIntifaceCentralDeviceConnector::CIntifaceCentralDeviceConnector() :
  CButtplugDeviceConnector()
{

}
CIntifaceCentralDeviceConnector::~CIntifaceCentralDeviceConnector()
{

}

//----------------------------------------------------------------------------------------
//
bool CIntifaceCentralDeviceConnector::CanConnect()
{
  if (!m_spClient->IsLoaded()) { return false; }

  if (!FindIntifaceProcess())
  {
    if (StartIntiface())
    {
      // we return true, because the engine is started but sadly we can't currently start
      // it with the server already running
      return true;
    }
  }
  else
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceCentralDeviceConnector::Connect()
{
  if (!m_spClient->IsLoaded()) { return false; }
  return m_spClient->Connect();
}

//----------------------------------------------------------------------------------------
//
void CIntifaceCentralDeviceConnector::Disconnect()
{
  if (m_spClient->IsLoaded())
  {
    m_spClient->Disconnect();
  }
}

//----------------------------------------------------------------------------------------
//
void CIntifaceCentralDeviceConnector::SlotProcessError(QProcess::ProcessError error)
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
bool CIntifaceCentralDeviceConnector::FindIntifaceProcess()
{
  qint32 iPid = FindProcessId("intiface_central");
  if (-1 == iPid)
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceCentralDeviceConnector::StartIntiface()
{
  const QString sPluginPath = FindPluginPath();

  auto pSetting = CDeviceSettingFactory::Setting<QString>(c_sIntifaceLocationSettingName);
  if (nullptr == pSetting)
  {
    qWarning() << "Could not load intiface install location setting.";
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
DECLARE_DEVICE_SETTING(struct SIntifacePort, c_sIntifacePortSettingName, quint16, 12345,
                       ::CreatePortWidget)
DECLARE_DEVICE_SETTING(struct SIntifaceInstallLocation, c_sIntifaceLocationSettingName, QString, QString(),
                       ::CreateInstallLocaionWidget)
