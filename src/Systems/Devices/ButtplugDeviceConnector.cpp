#include "ButtplugDeviceConnector.h"
#include "Application.h"
#include "ButtplugDeviceConnectorContext.h"
#include "ButtplugDeviceWrapper.h"
#include "DeviceSettings.h"

#if defined(Q_OS_ANDROID)
#include "Android/AndroidApplicationWindow.h"
#endif

#include "Systems/HelpFactory.h"

#include "Widgets/HelpOverlay.h"

#include "Utils/WidgetHelpers.h"

#include <QButtplugClient>
#include <QButtplugClientDevice>

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
      wpHelpFactory->RegisterHelp(c_sIntifacePortSettingHelpId, ":/devices/resources/help/settings/initface_port_setting_help.html");
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
      wpHelpFactory->RegisterHelp(c_sIntifaceInstallSettingHelpId, ":/devices/resources/help/settings/initface_install_setting_help.html");
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
#elif defined(Q_OS_ANDROID)
    auto activity = GetAndroidActivity();
    iRet = IsAppRunning(activity, sProcess) ? 0 : -1;
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    constexpr size_t c_iBuffSize = 512;
    char buf[c_iBuffSize];
    const QString sCommand = QString("pidof -s %1").arg(sProcess);
    FILE *cmd_pipe = popen(sCommand.toStdString().c_str(), "r");

    char* sRet = fgets(buf, c_iBuffSize, cmd_pipe);
    Q_UNUSED(sRet)
    pid_t pid = strtoul(buf, NULL, 10);

    pclose( cmd_pipe );

    iRet = static_cast<qint32>(pid);
#endif

    return iRet;
  }
}

//--------------------------------------------------------------------------------------
//
class CIntifaceEngineClientWrapper
{
public:
  CIntifaceEngineClientWrapper(std::function<void(quint32)> fnDeviceAdded,
                               std::function<void(quint32)> fnDeviceRemoved,
                               std::function<void()> fnDisconnected);
  ~CIntifaceEngineClientWrapper();

  bool Connect();
  void ConnectSignals();
  qint32 DeviceCount() const;
  void DeviceRemoved(quint32 iId);
  const std::map<quint32, QButtplugClientDevice>& Devices() const;
  bool Disconnect();
  bool IsConnected();
  bool StartScanning();
  bool StopScanning();

private:
  std::unique_ptr<QButtplugClient>         m_spClient;
  std::map<quint32, QButtplugClientDevice> m_deviceList;
  std::function<void(quint32)> m_fnDeviceAdded;
  std::function<void(quint32)> m_fnDeviceRemoved;
  std::function<void()> m_fnDisconnected;
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
                     [this](quint32) {
                       emit SignalDeviceCountChanged();
                     },
                     [this](quint32 id) {
                       emit m_pContext->SignalDeviceRemoved(QString::number(id));
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
  QStringList vsRet;
  for (const auto& [id, _] : m_spClient->Devices())
  {
    vsRet.push_back(QString::number(id));
  }
  return vsRet;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IDevice> CButtplugDeviceConnector::Device(const QString& sName) const
{
  auto it = m_spClient->Devices().find(static_cast<quint32>(sName.toLongLong()));
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
  std::vector<std::shared_ptr<IDevice>> ret;
  for (const auto& [_, device] : m_spClient->Devices())
  {
    ret.push_back(std::make_shared<CButtplugDeviceWrapper>(device, m_pContext));
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
bool CButtplugDeviceConnector::IsConnected() const
{
  return m_spClient->IsConnected();
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceConnector::StartScanning()
{
  m_spClient->StartScanning();
}

//----------------------------------------------------------------------------------------
//
void CButtplugDeviceConnector::StopScanning()
{
  m_spClient->StopScanning();
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
    std::function<void(quint32)> fnDeviceAdded,
    std::function<void(quint32)> fnDeviceRemoved,
    std::function<void()> fnDisconnected) :
  m_fnDeviceAdded(fnDeviceAdded),
  m_fnDeviceRemoved(fnDeviceRemoved),
  m_fnDisconnected(fnDisconnected)
{
}
CIntifaceEngineClientWrapper::~CIntifaceEngineClientWrapper()
{
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineClientWrapper::Connect()
{
  m_spClient.reset(new QButtplugClient());
  m_spClient->setClientName("JOIPEngine");
  m_spClient->setProtocolVersion(QtButtplug::AnyProtocolVersion);

  ConnectSignals();

  auto pSetting = CDeviceSettingFactory::Setting<quint16>(c_sIntifacePortSettingName);
  if (nullptr == pSetting)
  {
    return false;
  }

  m_spClient->connectToHost(QHostAddress::LocalHost, pSetting->GetValue());
  bool bOk = m_spClient->waitConnected(15000);

  if (bOk)
  {
    QObject::connect(m_spClient.get(), &QButtplugClient::disconnected, m_spClient.get(),
            [this]() {
      qDebug() << "Intiface Client Server disconnected";
      if (nullptr != m_fnDisconnected)
      {
        m_fnDisconnected();
      }
    });
  }
  return bOk;
}

//----------------------------------------------------------------------------------------
//
void CIntifaceEngineClientWrapper::ConnectSignals()
{
  QObject::connect(m_spClient.get(), &QButtplugClient::connected, m_spClient.get(),
          []() {
    qDebug() << "Intiface Client Server connected";
  });
  QObject::connect(m_spClient.get(), &QButtplugClient::connectionStateChanged, m_spClient.get(),
          [](QtButtplug::ConnectionState state) {
    qDebug() << "Intiface client connection state changed" << state;
  });
  QObject::connect(m_spClient.get(), &QButtplugClient::scanningFinished, m_spClient.get(),
          []() {
    qDebug() << "Intiface Client Scanning finished";
  });
  QObject::connect(m_spClient.get(), &QButtplugClient::errorRecieved, m_spClient.get(),
          [](QtButtplug::Error error) {
    qDebug() << "Intiface Client error recieved:" << QButtplugClient::errorString(error);
  });
  QObject::connect(m_spClient.get(), &QButtplugClient::deviceAdded, m_spClient.get(),
          [this](quint32 iId, QButtplugClientDevice device) {
    if (device.isValid())
    {
      qDebug() << "Intiface Client: Device added";
      m_deviceList.insert({iId, device});
      if (nullptr != m_fnDeviceAdded)
      {
        m_fnDeviceAdded(iId);
      }
    }
  });
  QObject::connect(m_spClient.get(), &QButtplugClient::deviceRemoved, m_spClient.get(),
          [this](quint32 iId) {
    qDebug() << "Intiface Client: Device removed";
    DeviceRemoved(iId);
  });
}

//----------------------------------------------------------------------------------------
//
qint32 CIntifaceEngineClientWrapper::DeviceCount() const
{
  if (nullptr == m_spClient) { return false; }
  return m_spClient->deviceCount();
}

//----------------------------------------------------------------------------------------
//
void CIntifaceEngineClientWrapper::DeviceRemoved(quint32 iId)
{
  auto it = m_deviceList.find(iId);
  if (m_deviceList.end() != it)
  {
    m_deviceList.erase(it);
    if (nullptr != m_fnDeviceRemoved)
    {
      m_fnDeviceRemoved(iId);
    }
  }
}

//----------------------------------------------------------------------------------------
//
const std::map<quint32, QButtplugClientDevice>&
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
    DeviceRemoved(m_deviceList.begin()->first);
  }

  m_spClient->disconnectFromHost();
  bool bOk = m_spClient->waitDisconnected();

  m_spClient.reset(nullptr);

  return bOk;
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineClientWrapper::IsConnected()
{
  return nullptr != m_spClient && m_spClient->connectionState() == QtButtplug::Connected;
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineClientWrapper::StartScanning()
{
  if (nullptr == m_spClient)
  {
    return false;
  }

  m_spClient->startScan();

  return m_spClient->isScanning();
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineClientWrapper::StopScanning()
{
  if (nullptr == m_spClient)
  {
    return false;
  }

  m_spClient->stopScan();

  return !m_spClient->isScanning();
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
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CIntifaceEngineDeviceConnector::Connect()
{
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
  m_spClient->Disconnect();

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
  return m_spClient->Connect();
}

//----------------------------------------------------------------------------------------
//
void CIntifaceCentralDeviceConnector::Disconnect()
{
  m_spClient->Disconnect();
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
  // we can't start a process on android
#ifndef Q_OS_ANDROID
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
#else
  return true;
#endif
}

//----------------------------------------------------------------------------------------
// registering settings here
DECLARE_DEVICE_SETTING(struct SIntifacePort, c_sIntifacePortSettingName, quint16, 12345,
                       ::CreatePortWidget)
DECLARE_DEVICE_SETTING(struct SIntifaceInstallLocation, c_sIntifaceLocationSettingName, QString, QString(),
                       ::CreateInstallLocaionWidget)
