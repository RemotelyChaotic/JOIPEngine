import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.1

FileBrowserForm {

    FileDialog {
        id: fileDialog
        title: qsTr("Choose a folder")
        selectExisting: true
        selectMultiple: false
        selectFolder: true
        onAccepted: {
            for (var i = 0; i < fileUrls.length; ++i) {
                var path = fileUrls[i].toString();
                // remove prefixed "file:///"
                path = path.replace(/^(file:\/{3})/,"");
                // unescape html codes like '%23' for '#'
                cleanPath = decodeURIComponent(path);
                pathTextField.text = cleanPath;
                break;
            }
        }
    }

    browseButton.onClicked: {
        fileDialog.open();
    }

    /*
    pathTextField.onEditingFinished: {
        console.log("")
    }*/
}
