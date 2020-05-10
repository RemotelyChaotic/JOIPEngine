import QtQuick 2.14

Item {
    id: registrator

    function componentLoaded()
    {
        root.numReadyComponents += 1;
    }
}
