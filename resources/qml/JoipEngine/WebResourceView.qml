import QtQuick 2.14
import QtQuick.Controls 2.14
import JOIP.core 1.5
import JOIP.db 1.1

Rectangle {
    id: webViewResource
    color: "transparent"

    property Resource resource: null
    property int state: Resource.Null
    property bool animated: false

    signal finishedPlaying();

    function pause()
    {
        if (animated && null !== webView && undefined !== webView)
        {
            webView.pause();
        }
    }

    function play()
    {
        if (animated && null !== webView && undefined !== webView)
        {
            webView.play();
        }
    }

    function stop()
    {
        if (animated && null !== webView && undefined !== webView)
        {
            webView.stop();
        }
    }

    onResourceChanged: {
        if (null !== resource && undefined !== resource)
        {
            state = Resource.Loading;
            if (resource.isLocal)
            {
                state = Resource.Null;
            }
            else
            {
                if (null !== webView && undefined !== webView)
                {
                    webViewResource.animated = resource.type !== Resource.Image;
                    webView.source = resource.path;
                }
                else
                {
                    console.error(qsTr("Cannot display remote-resource because the webView was not created."));
                }
            }
        }
        else
        {
            state = Resource.Null;
        }
    }

    property var webViewComponent: null
    property WebResourceComponent webView: null
    function createWebview()
    {
        webViewResource.webViewComponent = Qt.createComponent("WebResourceComponent.qml");
        if (webViewResource.webViewComponent.status === Component.Ready)
        {
            webViewResource.webView = webViewResource.webViewComponent.createObject(webViewResource);
            if (webViewResource.webView === null) {
                console.error(qsTr("Error creating WebView object"));
            }
            else
            {
                webViewResource.webView.muted = Settings.muted;
                webViewResource.webView.volume = Settings.volume;
            }
        }
    }

    Component.onCompleted: {
        if (!Settings.offline) {
            createWebview();
        }
    }
}
