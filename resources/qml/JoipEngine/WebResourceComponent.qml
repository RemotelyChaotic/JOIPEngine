import QtQuick 2.14
import QtQuick.Controls 2.14
import QtWebView 1.14
import JOIP.core 1.1
import JOIP.db 1.1

Rectangle {
    id: webResourceComponentRoot
    anchors.centerIn: parent
    width: (webResourceComponentRoot.parent.state === Resource.Loaded) ? parent.width : 0
    height: (webResourceComponentRoot.parent.state === Resource.Loaded) ? parent.height : 0
    color: "transparent"

    property string source: ""
    property bool muted: Settings.muted
    property real volume: Settings.volume
    property alias view: webViewInternal

    onMutedChanged: {
        if (webResourceComponentRoot.parent.state === Resource.Loaded)
        {
            setMuted(Settings.muted);
        }
    }
    onVolumeChanged: {
        if (webResourceComponentRoot.parent.state === Resource.Loaded)
        {
            setVolume(Settings.volume);
        }
    }

    function pause()
    {
        webViewInternal.runJavaScript(
                    "var video = document.querySelector('video');" +
                    "if (video !== null) {" +
                    "  if (!video.paused) {" +
                    "    video.pause();" +
                    "  }" +
                    "}");
    }

    function play()
    {
        webViewInternal.runJavaScript(
                    "var video = document.querySelector('video');" +
                    "if (video !== null) {" +
                    "  if (video.paused) {" +
                    "    video.play(); " +
                    "  }" +
                    "}");
    }

    function stop()
    {
        webViewInternal.runJavaScript(
                    "var video = document.querySelector('video'); " +
                    "if (video !== null) { " +
                    "  if (!video.paused) { " +
                    "    video.pause(); " +
                    "    video.load(); " +
                    "  } else { " +
                    "    video.load(); " +
                    "  } " +
                    "  video.pause(); " +
                    "}"
                    );
    }

    function setVolume(dVolume)
    {
        webViewInternal.runJavaScript(
                    "var video = document.querySelector('video');" +
                    "if (video !== null) {" +
                    "  video.volume = " + dVolume + ";" +
                    "}");
    }

    function setMuted(bMuted)
    {
        webViewInternal.runJavaScript(
                    "var video = document.querySelector('video');" +
                    "if (video !== null) {" +
                    "  video.muted = " + (bMuted ? "true" : "false") + ";" +
                    "}");
    }

    onSourceChanged: {
        if ("" !== source)
        {
            webViewInternal.url = source;
        }
        else
        {
            webResourceComponentRoot.parent.state = Resource.Null;
        }
    }

    Behavior on height {
        animation: ParallelAnimation {
            NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
        }
    }
    Behavior on width {
        animation: ParallelAnimation {
            NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
        }
    }

    WebView {
        id: webViewInternal
        anchors.fill: parent

        onLoadingChanged: {
            if (WebView.LoadSucceededStatus === loadRequest.status)
            {
                webResourceComponentRoot.parent.state = Resource.Loaded;
                webViewInternal.runJavaScript(
                            "var video = document.querySelector('video');" +
                            "if (video !== null) {" +
                            "  video.controls = false;" +
                            "}");
                if (webResourceComponentRoot.parent.animated)
                {
                    setVolume(webResourceComponentRoot.volume);
                    setMuted(webResourceComponentRoot.muted);
                    // video ended
                    webViewInternal.runJavaScript(
                                "var video = document.querySelector('video');" +
                                "if (video !== null) {" +
                                "  return video.onended;" +
                                "} else { return null; }", function(onended) {
                                    console.log(onended);
                                });
                }
            }
            else if (WebView.LoadFailedStatus === loadRequest.status)
            {
                webResourceComponentRoot.parent.state = Resource.Error;
                console.error(qsTr("Error loading remote resource: %1.")
                              .arg(loadRequest.errorString));
            }
            else if (WebView.LoadStartedStatus === loadRequest.status)
            {
                webResourceComponentRoot.parent.state = Resource.Loading;
            }
        }
    }
}
