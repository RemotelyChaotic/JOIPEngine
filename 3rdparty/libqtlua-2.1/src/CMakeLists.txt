cmake_policy(SET CMP0100 NEW)

SET(QTLUA_LIB_SRCS )

# Generate moc files

QT5_WRAP_CPP(MOC_OUTFILES QtLua/qtluaconsole.hh        OPTIONS -fQtLua/Console)
QT5_WRAP_CPP(MOC_OUTFILES QtLua/qtluauseritemselectionmodel.hh OPTIONS -fQtLua/UserItemSelectionModel)
QT5_WRAP_CPP(MOC_OUTFILES QtLua/qtluauseritemmodel.hh  OPTIONS -fQtLua/UserItemModel)
QT5_WRAP_CPP(MOC_OUTFILES QtLua/qtlualuamodel.hh       OPTIONS  -fQtLua/LuaModel)
QT5_WRAP_CPP(MOC_OUTFILES QtLua/qtluatabletreemodel.hh OPTIONS -fQtLua/TableTreeModel)
QT5_WRAP_CPP(MOC_OUTFILES QtLua/qtluaitemviewdialog.hh OPTIONS -fQtLua/ItemViewDialog)
QT5_WRAP_CPP(MOC_OUTFILES QtLua/qtluatablegridmodel.hh OPTIONS -fQtLua/TableGridModel)
QT5_WRAP_CPP(MOC_OUTFILES QtLua/qtluastate.hh          OPTIONS -fQtLua/State)
QT5_WRAP_CPP(MOC_OUTFILES qtluaqtlib.hh)

ADD_LIBRARY(qtlua STATIC

            qtluaconsole.cc qtluaenum.cc
            qtluaenumiterator.cc qtluafunction.cc
            qtluauseritem.cc qtluauseritemmodel.cc
            qtluauseritemselectionmodel.cc
            qtluauserlistitem.cc qtlualistiterator.cc
            qtluamember.cc qtluametacache.cc
            qtluamethod.cc qtluaplugin.cc qtluaproperty.cc
            qtluaqmetaobjecttable.cc qtluaqmetavalue.cc
            qtluaqmetaobjectwrapper.cc qtlualuamodel.cc
            qtluaqobjectiterator.cc qtluaqobjectwrapper.cc
            qtluaqtlib.cc qtluastate.cc
            qtluaitemviewdialog.cc qtluatablegridmodel.cc
            qtluatableiterator.cc qtluatabletreekeys.cc
            qtluatabletreemodel.cc qtluauserdata.cc
            qtluavaluebase.cc qtluavalue.cc
            qtluavalueref.cc qtluadispatchproxy.cc

            ${MOC_OUTFILES})

TARGET_LINK_LIBRARIES(qtlua Qt5::Core Qt5::Widgets lua_static)
TARGET_INCLUDE_DIRECTORIES(qtlua PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INC}>
)

INSTALL(TARGETS qtlua
	LIBRARY DESTINATION ${INSTALL_LIB}
	ARCHIVE DESTINATION ${INSTALL_LIB})

INSTALL(FILES
        QtLua/Function           QtLua/qtluafunction.hh        QtLua/qtluafunction.hxx 
        QtLua/Console            QtLua/qtluaconsole.hh         QtLua/qtluaconsole.hxx 
        QtLua/TableTreeModel     QtLua/qtluatabletreemodel.hh  QtLua/qtluatabletreemodel.hxx
        QtLua/ItemViewDialog     QtLua/qtluaitemviewdialog.hh  QtLua/qtluaitemviewdialog.hxx
        QtLua/TableGridModel     QtLua/qtluatablegridmodel.hh  QtLua/qtluatablegridmodel.hxx
        QtLua/LuaModel           QtLua/qtlualuamodel.hh        QtLua/qtlualuamodel.hxx
        QtLua/UserItem           QtLua/qtluauseritem.hh        QtLua/qtluauseritem.hxx
        QtLua/UserListItem       QtLua/qtluauserlistitem.hh    QtLua/qtluauserlistitem.hxx 
        QtLua/UserItemModel      QtLua/qtluauseritemmodel.hh   QtLua/qtluauseritemmodel.hxx
        QtLua/UserItemSelectionModel QtLua/qtluauseritemselectionmodel.hh QtLua/qtluauseritemselectionmodel.hxx 
        QtLua/Iterator           QtLua/qtluaiterator.hh        QtLua/qtluaiterator.hxx 
        QtLua/Plugin             QtLua/qtluaplugin.hh          QtLua/qtluaplugin.hxx 
        QtLua/State              QtLua/qtluastate.hh           QtLua/qtluastate.hxx 
        QtLua/UserData           QtLua/qtluauserdata.hh        QtLua/qtluauserdata.hxx 
        QtLua/UserObject         QtLua/qtluauserobject.hh      QtLua/qtluauserobject.hxx 
        QtLua/ValueBase          QtLua/qtluavaluebase.hh       QtLua/qtluavaluebase.hxx
        QtLua/Value              QtLua/qtluavalue.hh           QtLua/qtluavalue.hxx 
        QtLua/ValueRef           QtLua/qtluavalueref.hh        QtLua/qtluavalueref.hxx 
        QtLua/Ref                QtLua/qtluaref.hh 
        QtLua/String             QtLua/qtluastring.hh          QtLua/qtluastring.hxx 
        QtLua/QHashProxy         QtLua/qtluaqhashproxy.hh      QtLua/qtluaqhashproxy.hxx 
        QtLua/QVectorProxy       QtLua/qtluaqvectorproxy.hh    QtLua/qtluaqvectorproxy.hxx 
        QtLua/QListProxy         QtLua/qtluaqlistproxy.hh      QtLua/qtluaqlistproxy.hxx 
        QtLua/QLinkedListProxy   QtLua/qtluaqlinkedlistproxy.hh QtLua/qtluaqlinkedlistproxy.hxx 
        QtLua/ArrayProxy         QtLua/qtluaarrayproxy.hh      QtLua/qtluaarrayproxy.hxx 
        QtLua/MetaType           QtLua/qtluametatype.hh        QtLua/qtluametatype.hxx 
        QtLua/DispatchProxy      QtLua/qtluadispatchproxy.hh   QtLua/qtluadispatchproxy.hxx

        DESTINATION ${INSTALL_INC}/QtLua)

