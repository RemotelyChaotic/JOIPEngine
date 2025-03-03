/*
    This file is part of LibQtLua.

    LibQtLua is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LibQtLua is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LibQtLua.  If not, see <http://www.gnu.org/licenses/>.

    Copyright (C) 2008, Alexandre Becoulet <alexandre.becoulet@free.fr>

    Fork
    Copyright (C) 2015 (Li, Kwue-Ron) <likwueron@gmail.com>
*/

#include <QDebug>
#include <QObject>
#include <QMetaObject>
#include <QWidget>

#include <internal/QObjectWrapper>

#include <internal/QMetaValue>
#include <internal/Method>
#include <internal/MetaCache>
#include <internal/QObjectIterator>

#define assert_do(x) { bool res_ = (x); assert (((void)#x, res_)); Q_UNUSED(res_) }

namespace QtLua {

  static const int destroyindex = QObject::staticMetaObject.indexOfSignal("destroyed()");

  QObjectWrapper::QObjectWrapper(State *ls, QObject *obj)
    : _ls(ls),
      _obj(obj),
      _lua_next_slot(1),
      _reparent(false),
      _delete(obj && obj->parent())
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapper object created" << _obj;
#endif

    if (_obj)
      {
	assert_do(QMetaObject::connect(obj, destroyindex, this, metaObject()->methodCount() + 0));

	ls->_whash.insert(obj, this);

	// increment reference count since we are bound to a qobject
	_inc();
      }
  }

  Ref<QObjectWrapper> QObjectWrapper::get_wrapper(State *ls, QObject *obj)
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapper object get" << obj;
#endif

    if (obj)
      {
	wrapper_hash_t::iterator i = ls->_whash.find(obj);

	if (i != ls->_whash.end())
	  return *i.value();
      }

    QObjectWrapper::ptr qow = QTLUA_REFNEW(QObjectWrapper, ls, obj);

    return qow;
  }

  Ref<QObjectWrapper> QObjectWrapper::get_wrapper(State *ls, QObject *obj, bool reparent, bool delete_)
  {
    QObjectWrapper::ptr qow = get_wrapper(ls, obj);

    qow->_reparent = reparent;
    qow->_delete = delete_;

    return qow;
  }

  void QObjectWrapper::obj_destroyed()
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapped object has been destroyed" << _obj;
#endif
    assert(_obj = sender());

    assert_do(_ls->_whash.remove(_obj));
    _obj = 0;
    _drop();
  }

  QObjectWrapper::~QObjectWrapper()
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapper object detructor" << _obj;
#endif

    if (_obj)
      {
	assert_do(_ls->_whash.remove(_obj));

	assert_do(QMetaObject::disconnect(_obj, destroyindex, this, metaObject()->methodCount() + 0));

	_lua_disconnect_all();

	if (!_obj->parent() && _delete)
	  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
	    qDebug() << "wrapped object delete" << _obj;
#endif
	    delete _obj;
	  }
      }
  }

  void QObjectWrapper::ref_single()
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapper refdrop" << _delete << _obj;
#endif

    if ((_obj && !_obj->parent()) && _delete)
      _drop();
  }

  int QObjectWrapper::qt_metacall(QMetaObject::Call c, int id, void **qt_args)
  {
    id = QObject::qt_metacall(c, id, qt_args);

    if (id < 0 || c != QMetaObject::InvokeMetaMethod)
      return id;

    if (id == 0)
      {
	// slot 0 is reserved for object.destroyed() signal
	obj_destroyed();
	return -1;
      }

    if (!_obj)
      return -1;

    lua_slots_hash_t::iterator i = _lua_slots.find(id);
    assert(i != _lua_slots.end());

    Value::List lua_args;

    // first arg is sender object
    assert(_obj == sender());
    lua_args.push_back(Value(_ls, QObjectWrapper::get_wrapper(_ls, _obj)));

    // push more args from parameter type informations
    QMetaMethod mm = _obj->metaObject()->method(i.value()._sigindex);

    foreach(const QByteArray &pt, mm.parameterTypes())
      {
	qt_args++;
	lua_args.push_back(QMetaValue::raw_get_object(_ls, QMetaType::type(pt.constData()), *qt_args));
      }

    try {
      i.value()._value.call(lua_args);
    } catch (const String &err) {
      qDebug() << "Error executing lua slot:" << err;
    }

    return -1;
  }

  void QObjectWrapper::_lua_connect(int sigindex, const Value &value)
  {
    get_object();

    switch (value.type())
      {
      case Value::TUserData:
      case Value::TFunction: {
	int slot_id;

	do {
	  slot_id = _lua_next_slot++;
	} while (_lua_slots.contains(slot_id));

	if (QMetaObject::connect(_obj, sigindex, this, metaObject()->methodCount() + slot_id))
	  {
	    _lua_slots.insert(slot_id, LuaSlot(value, sigindex));
	    return;
	  }

  QTLUA_THROW_NOARG(QtLua::QObjectWrapper, "Failed to connect the Qt signal to a lua function.");
      }

      default:
	QTLUA_THROW(QtLua::QObjectWrapper, "Can not connect a `lua::%' lua value to a Qt signal.",
		    .arg(value.type_name()));
      }
  }

  bool QObjectWrapper::_lua_disconnect(int sigindex, const Value &value)
  {
    if (!_obj)
      return false;

    lua_slots_hash_t::iterator i;

    for (i = _lua_slots.begin(); i != _lua_slots.end(); )
      {
	if (i.value()._sigindex == sigindex && value == i.value()._value)
	  {
	    bool ok = QMetaObject::disconnect(_obj, sigindex, this, metaObject()->methodCount() + i.key());
      assert(ok); Q_UNUSED(ok)
	    _lua_next_slot = std::min(_lua_next_slot, i.key());
	    i = _lua_slots.erase(i);
	    return true;
	  }
	else
	  ++i;
      }

    return false;
  }

  void QObjectWrapper::_lua_disconnect_all(int sigindex)
  {
    if (!_obj)
      return;

    lua_slots_hash_t::iterator i;

    for (i = _lua_slots.begin(); i != _lua_slots.end(); )
      {
	if (i.value()._sigindex == sigindex)
	  {
	    bool ok = QMetaObject::disconnect(_obj, sigindex, this, metaObject()->methodCount() + i.key());
      assert(ok); Q_UNUSED(ok)
	    _lua_next_slot = std::min(_lua_next_slot, i.key());
	    i = _lua_slots.erase(i);
	  }
	else
	  ++i;
      }
  }

  void QObjectWrapper::_lua_disconnect_all()
  {
    if (!_obj)
      return;

    lua_slots_hash_t::iterator i;

    for (i = _lua_slots.begin(); i != _lua_slots.end(); )
      {
	bool ok = QMetaObject::disconnect(_obj, i.value()._sigindex, this, metaObject()->methodCount() + i.key());
  assert(ok); Q_UNUSED(ok)
	++i;
      }
    _lua_slots.clear();
    _lua_next_slot = 1;
  }

  QObject * QObjectWrapper::get_child(QObject &obj, const String &name)
  {
    foreach (QObject *child, obj.children())
      if (QObjectWrapper::qobject_name(*child) == name)
	return child;
    return 0;
  }

  Value QObjectWrapper::meta_index(State *ls, const Value &key)
  {
    QObject &obj = get_object();
    String skey = key.to_string();

    // handle children access
    if (QObject *child = get_child(obj, skey))
      return Value(ls, QObjectWrapper::get_wrapper(ls, child));

    // fallback to member read access
    MetaCache &mc = MetaCache::get_meta(obj);
    Member::ptr m = mc.get_member(skey);

    if(m.valid())
      return m->access(*this);
    else
      {
        int index = mc.get_index_getDP();
        if(index != -1)
          {
            QVariant dp;
            QByteArray name = skey;
            void *argv[2] = {Q_RETURN_ARG(QVariant, dp).data(), Q_ARG(QByteArray, name).data()};
            obj.qt_metacall(QMetaObject::InvokeMetaMethod, index, argv);
            return Value(ls, dp);
          }
        else if(mc.can_auto_property()) {
            return Value(ls, obj.property(skey));
        }
        else return Value(ls);
      }
  }

  void QObjectWrapper::reparent(QObject *parent)
  {
    assert(_obj);

    if (!_reparent)
      QTLUA_THROW(QtLua::QObjectWrapper, "Parent change disallowed for the `%' QObject.",
		  .arg(QObjectWrapper::qobject_name(*_obj)));

    if (!_obj->isWidgetType() || (parent && !parent->isWidgetType()))
      _obj->setParent(parent);
    else
      qobject_cast<QWidget*>(_obj)->setParent(qobject_cast<QWidget*>(parent));
  }

  void QObjectWrapper::meta_newindex(State *ls, const Value &key, const Value &value)
  {
    QObject &obj = get_object();
    String skey = key.to_string();

    // handle existing children access
    if (QObject *cobj = get_child(obj, skey))
      {
        //old child
	QObjectWrapper::ptr cw = get_wrapper(ls, cobj);

	if (value.is_nil())
	  {
	    cw->reparent(0);
	    return;
	  }
        //new child
	QObjectWrapper::ptr vw = value.to_userdata_cast<QObjectWrapper>();
	QObject &vobj = vw->get_object();

	cw->reparent(0);
	vobj.setObjectName(skey.to_qstring());
	vw->reparent(&obj);
	return;
      }
    else
      {
        MetaCache &mc = MetaCache::get_meta(obj);
	// fallback to member write access
        Member::ptr m = mc.get_member(skey);

	if (m.valid())
	  {
	    m->assign(*this, value);
	    return;
	  }
      }

    switch(value.type()) {
    case ValueBase::TUserData: {
        // child insertion
        QObjectWrapper::ptr vw = value.to_userdata_cast<QObjectWrapper>();
        QObject &vobj = vw->get_object();

        vobj.setObjectName(skey.to_qstring());
        vw->reparent(&obj);
    break; }
    case ValueBase::TNumber:
    case ValueBase::TBool:
    case ValueBase::TString:
    case ValueBase::TTable: {
        MetaCache &mc = MetaCache::get_meta(obj);
        int index = mc.get_index_setDP();
        if(index != -1)
        {
          QVariant dp = value.to_qvariant();
          QByteArray name = skey;
          void *argv[3] = {0x0, Q_ARG(QByteArray, name).data(), Q_ARG(QVariant, dp).data()};
          obj.qt_metacall(QMetaObject::InvokeMetaMethod, index, argv);
        }
        else if(mc.can_auto_property()) {
            obj.setProperty(skey, value.to_qvariant());
        }
    break; }
    default:
        QTLUA_THROW(QtLua::QObjectWrapper, "Cannot assign value type `%' to QObject",
                    .arg(value.type_name()));
        break;
    }
  }

  Ref<Iterator> QObjectWrapper::new_iterator(State *ls)
  {
    get_object();
    return QTLUA_REFNEW(QObjectIterator, ls, *this);
  }

  bool QObjectWrapper::support(Value::Operation c) const
  {
    switch (c)
      {
      case Value::OpIndex:
      case Value::OpNewindex:
      case Value::OpIterate:
	return true;
      default:
	return false;
      }
  }

  String QObjectWrapper::get_type_name() const
  {
    return _obj ? MetaCache::get_meta_name(_obj->metaObject()) : "";
  }

  String QObjectWrapper::get_value_str() const
  {
    if(!_obj) return "(deleted)";
    QString result;
    int index = MetaCache::get_index_toString(*_obj);
    if(index != -1)
      {
        void *args[1] = {static_cast<void*>(&result)};
        _obj->qt_metacall(QMetaObject::InvokeMetaMethod, index, args);

      }
    else
      {
        result = QString("0x%1(%2)")
                .arg((qulonglong)_obj, 0, 16)
                .arg(qobject_name(*_obj).constData());
      }
    return result;
  }

  void QObjectWrapper::completion_patch(String &, String &entry, int &)
  {
    if (_obj)
      entry += ".";
  }

  String QObjectWrapper::qobject_name(QObject &obj)
  {
    if (obj.objectName().isEmpty())
      {
	QString name;

  name.asprintf("%s_%llx", MetaCache::get_meta_name(obj.metaObject()).constData(),
               reinterpret_cast<unsigned long long>(&obj));
	obj.setObjectName(name.toLower());
      }

    return obj.objectName();
  }

}

