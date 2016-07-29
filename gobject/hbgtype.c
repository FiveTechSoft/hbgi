/*
 * hbgi source code
 * Core code
 *
 * Copyright 2014-2016 Phil Krylov <phil.krylov a t gmail.com>
 *
 * Most of the logic in this file is based on pygtype.c from pygobject
 * library:
 *
 * pygtk- Python bindings for the GTK toolkit.
 * Copyright (C) 1998-2003  James Henstridge
 *
 *   pygtype.c: glue code to wrap the GType code.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include <glib-object.h>

#include <hbapi.h>
#include <hbapicls.h>
#include <hbapierr.h>
#include <hbapiitm.h>
#include <hbstack.h>
#define _HB_API_INTERNAL_ /* for hb_vmEval() */
#include <hbvm.h>

#include "hbgihb.h"

#include "hbgobject.h"
#include "hbgpointer.h"
#include "hbgtype.h"

#define HBGTYPE_IVAR_GTYPE 1
#define HBGTYPE_IVAR_COUNT 1

/* -------------- __gtype__ objects ---------------------------- */

HB_USHORT HbGTypeWrapper_Type;

#if 0
typedef struct {
    PyObject_HEAD
    GType type;
} PyGTypeWrapper;

PYGLIB_DEFINE_TYPE("gobject.GType", PyGTypeWrapper_Type, PyGTypeWrapper);

static PyObject*
pyg_type_wrapper_richcompare(PyObject *self, PyObject *other, int op)
{
    if (Py_TYPE(self) == Py_TYPE(other) && Py_TYPE(self) == &PyGTypeWrapper_Type)
        return _pyglib_generic_long_richcompare(((PyGTypeWrapper*)self)->type,
                                                ((PyGTypeWrapper*)other)->type,
                                                op);
    else {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
}

static long
pyg_type_wrapper_hash(PyGTypeWrapper *self)
{
    return (long)self->type;
}

static PyObject *
pyg_type_wrapper_repr(PyGTypeWrapper *self)
{
    char buf[80];
    const gchar *name = g_type_name(self->type);

    g_snprintf(buf, sizeof(buf), "<GType %s (%lu)>",
	       name?name:"invalid", (unsigned long int) self->type);
    return PYGLIB_PyUnicode_FromString(buf);
}

static void
pyg_type_wrapper_dealloc(PyGTypeWrapper *self)
{
    PyObject_DEL(self);
}

static GQuark
_pyg_type_key(GType type) {
    GQuark key;

    if (g_type_is_a(type, G_TYPE_INTERFACE)) {
        key = pyginterface_type_key;
    } else if (g_type_is_a(type, G_TYPE_ENUM)) {
        key = pygenum_class_key;
    } else if (g_type_is_a(type, G_TYPE_FLAGS)) {
        key = pygflags_class_key;
    } else if (g_type_is_a(type, G_TYPE_POINTER)) {
        key = pygpointer_class_key;
    } else if (g_type_is_a(type, G_TYPE_BOXED)) {
        key = pygboxed_type_key;
    } else {
        key = pygobject_class_key;
    }

    return key;
}

static PyObject *
_wrap_g_type_wrapper__get_pytype(PyGTypeWrapper *self, void *closure)
{
    GQuark key;
    PyObject *py_type;

    key = _pyg_type_key(self->type);

    py_type = g_type_get_qdata(self->type, key);
    if (!py_type)
      py_type = Py_None;

    Py_INCREF(py_type);
    return py_type;
}

static int
_wrap_g_type_wrapper__set_pytype(PyGTypeWrapper *self, PyObject* value, void *closure)
{
    GQuark key;
    PyObject *py_type;

    key = _pyg_type_key(self->type);

    py_type = g_type_get_qdata(self->type, key);
    Py_CLEAR(py_type);
    if (value == Py_None)
	g_type_set_qdata(self->type, key, NULL);
    else if (PyType_Check(value)) {
	Py_INCREF(value);
	g_type_set_qdata(self->type, key, value);
    } else {
	PyErr_SetString(PyExc_TypeError, "Value must be None or a type object");
	return -1;
    }

    return 0;
}

static PyObject *
_wrap_g_type_wrapper__get_name(PyGTypeWrapper *self, void *closure)
{
   const char *name = g_type_name(self->type);
   return PYGLIB_PyUnicode_FromString(name ? name : "invalid");
}

static PyObject *
_wrap_g_type_wrapper__get_parent(PyGTypeWrapper *self, void *closure)
{
   return pyg_type_wrapper_new(g_type_parent(self->type));
}

static PyObject *
_wrap_g_type_wrapper__get_fundamental(PyGTypeWrapper *self, void *closure)
{
   return pyg_type_wrapper_new(g_type_fundamental(self->type));
}

static PyObject *
_wrap_g_type_wrapper__get_children(PyGTypeWrapper *self, void *closure)
{
  guint n_children, i;
  GType *children;
  PyObject *retval;

  children = g_type_children(self->type, &n_children);

  retval = PyList_New(n_children);
  for (i = 0; i < n_children; i++)
      PyList_SetItem(retval, i, pyg_type_wrapper_new(children[i]));
  g_free(children);

  return retval;
}

static PyObject *
_wrap_g_type_wrapper__get_interfaces(PyGTypeWrapper *self, void *closure)
{
  guint n_interfaces, i;
  GType *interfaces;
  PyObject *retval;

  interfaces = g_type_interfaces(self->type, &n_interfaces);

  retval = PyList_New(n_interfaces);
  for (i = 0; i < n_interfaces; i++)
      PyList_SetItem(retval, i, pyg_type_wrapper_new(interfaces[i]));
  g_free(interfaces);

  return retval;
}

static PyObject *
_wrap_g_type_wrapper__get_depth(PyGTypeWrapper *self, void *closure)
{
  return PYGLIB_PyLong_FromLong(g_type_depth(self->type));
}

static PyGetSetDef _PyGTypeWrapper_getsets[] = {
    { "pytype", (getter)_wrap_g_type_wrapper__get_pytype, (setter)_wrap_g_type_wrapper__set_pytype },
    { "name",  (getter)_wrap_g_type_wrapper__get_name, (setter)0 },
    { "fundamental",  (getter)_wrap_g_type_wrapper__get_fundamental, (setter)0 },
    { "parent",  (getter)_wrap_g_type_wrapper__get_parent, (setter)0 },
    { "children",  (getter)_wrap_g_type_wrapper__get_children, (setter)0 },
    { "interfaces",  (getter)_wrap_g_type_wrapper__get_interfaces, (setter)0 },
    { "depth",  (getter)_wrap_g_type_wrapper__get_depth, (setter)0 },
    { NULL, (getter)0, (setter)0 }
};

static PyObject*
_wrap_g_type_is_interface(PyGTypeWrapper *self)
{
    return PyBool_FromLong(G_TYPE_IS_INTERFACE(self->type));
}

static PyObject*
_wrap_g_type_is_classed(PyGTypeWrapper *self)
{
    return PyBool_FromLong(G_TYPE_IS_CLASSED(self->type));
}

static PyObject*
_wrap_g_type_is_instantiatable(PyGTypeWrapper *self)
{
    return PyBool_FromLong(G_TYPE_IS_INSTANTIATABLE(self->type));
}

static PyObject*
_wrap_g_type_is_derivable(PyGTypeWrapper *self)
{
    return PyBool_FromLong(G_TYPE_IS_DERIVABLE(self->type));
}

static PyObject*
_wrap_g_type_is_deep_derivable(PyGTypeWrapper *self)
{
    return PyBool_FromLong(G_TYPE_IS_DEEP_DERIVABLE(self->type));
}

static PyObject*
_wrap_g_type_is_abstract(PyGTypeWrapper *self)
{
    return PyBool_FromLong(G_TYPE_IS_ABSTRACT(self->type));
}

static PyObject*
_wrap_g_type_is_value_abstract(PyGTypeWrapper *self)
{
    return PyBool_FromLong(G_TYPE_IS_VALUE_ABSTRACT(self->type));
}

static PyObject*
_wrap_g_type_is_value_type(PyGTypeWrapper *self)
{
    return PyBool_FromLong(G_TYPE_IS_VALUE_TYPE(self->type));
}

static PyObject*
_wrap_g_type_has_value_table(PyGTypeWrapper *self)
{
    return PyBool_FromLong(G_TYPE_HAS_VALUE_TABLE(self->type));
}

static PyObject*
_wrap_g_type_from_name(PyGTypeWrapper *_, PyObject *args)
{
    char *type_name;
    GType type;

    if (!PyArg_ParseTuple(args, "s:GType.from_name", &type_name))
	return NULL;

    type = _pyg_type_from_name(type_name);
    if (type == 0) {
	PyErr_SetString(PyExc_RuntimeError, "unknown type name");
	return NULL;
    }

    return pyg_type_wrapper_new(type);
}

static PyObject*
_wrap_g_type_is_a(PyGTypeWrapper *self, PyObject *args)
{
    PyObject *gparent;
    GType parent;

    if (!PyArg_ParseTuple(args, "O:GType.is_a", &gparent))
	return NULL;
    else if ((parent = pyg_type_from_object(gparent)) == 0)
	return NULL;

    return PyBool_FromLong(g_type_is_a(self->type, parent));
}

static PyMethodDef _PyGTypeWrapper_methods[] = {
    { "is_interface", (PyCFunction)_wrap_g_type_is_interface, METH_NOARGS },
    { "is_classed", (PyCFunction)_wrap_g_type_is_classed, METH_NOARGS },
    { "is_instantiatable", (PyCFunction)_wrap_g_type_is_instantiatable, METH_NOARGS },
    { "is_derivable", (PyCFunction)_wrap_g_type_is_derivable, METH_NOARGS },
    { "is_deep_derivable", (PyCFunction)_wrap_g_type_is_deep_derivable, METH_NOARGS },
    { "is_abstract", (PyCFunction)_wrap_g_type_is_abstract, METH_NOARGS },
    { "is_value_abstract", (PyCFunction)_wrap_g_type_is_value_abstract, METH_NOARGS },
    { "is_value_type", (PyCFunction)_wrap_g_type_is_value_type, METH_NOARGS },
    { "has_value_table", (PyCFunction)_wrap_g_type_has_value_table, METH_NOARGS },
    { "from_name", (PyCFunction)_wrap_g_type_from_name, METH_VARARGS | METH_STATIC },
    { "is_a", (PyCFunction)_wrap_g_type_is_a, METH_VARARGS },
    { NULL,  0, 0 }
};

static int
pyg_type_wrapper_init(PyGTypeWrapper *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", NULL };
    PyObject *py_object;
    GType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O:GType.__init__",
				     kwlist, &py_object))
        return -1;

    if (!(type = pyg_type_from_object(py_object)))
	return -1;

    self->type = type;

    return 0;
}
#endif

/**
 * hbg_type_wrapper_new:
 * type: a GType
 *
 * Creates a Harbour wrapper for a GType.
 *
 * Returns: the Harbour wrapper.
 */
PHB_ITEM
hbg_type_wrapper_new(GType type)
{
    PHB_ITEM self;

    self = hbgi_hb_clsInst(HbGTypeWrapper_Type);
    if (self == NULL)
	return NULL;

    hb_arraySetNLL(self, HBGTYPE_IVAR_GTYPE, type);
    return self;
}


/**
 * hbg_type_from_object_strict:
 * obj: a Harbour item
 * strict: if set to TRUE, raises an exception if it can't perform the
 *         conversion
 *
 * converts a Harbour item to a GType.  If strict is set, raises an 
 * exception if it can't perform the conversion, otherwise returns
 * HB_TYPE_ITEM.
 *
 * Returns: the corresponding GType, or 0 on error.
 */

GType
hbg_type_from_object_strict(PHB_ITEM obj, gboolean strict)
{
    PHB_ITEM gtype;
    GType type;

    /* NULL check */
    if (!obj) {
        hb_errRT_BASE_SubstR(EG_DATATYPE, 50080, "can't get type from NULL object", "hbgobject", HB_ERR_ARGS_BASEPARAMS);
	return 0;
    }

    /* map some standard types to primitive GTypes ... */
    if (HB_IS_NIL(obj))
	return G_TYPE_NONE;
    if (HB_IS_INTEGER(obj))
	return G_TYPE_INT;
    if (HB_IS_LOGICAL(obj))
	return G_TYPE_BOOLEAN;
    if (HB_IS_LONG(obj))
	return G_TYPE_LONG;
    if (HB_IS_DOUBLE(obj))
	return G_TYPE_DOUBLE;

    if (HB_IS_OBJECT(obj) && hb_objGetClass(obj) == HbGTypeWrapper_Type) {
	return (GType)hb_arrayGetNLL(obj, HBGTYPE_IVAR_GTYPE);
    }

    /* handle strings */
    if (HB_IS_STRING(obj)) {
	const char *name = hb_itemGetCPtr(obj);

	type = _hbg_type_from_name(name);
	if (type != 0) {
	    return type;
	}
    }

    /* finally, look for a __gtype__ attribute on the object */
    gtype = hb_objSendMsg(obj, "__gtype__", 0);

    if (gtype) {
        if (HB_IS_OBJECT(gtype) && hb_objGetClass(gtype) == HbGTypeWrapper_Type) {
	    return (GType)hb_arrayGetNLL(gtype, HBGTYPE_IVAR_GTYPE);
        } else if (HB_IS_NUMINT(gtype)) {
            return (GType)hb_itemGetNLL(gtype);
        }
    }

    /* Some API like those that take GValues can hold a Harbour item as
     * a pointer.  This is potentially dangerous because everything is
     * passed in as a PHB_ITEM so we can't actually type check it.  Only
     * fallback to HB_TYPE_ITEM if strict checking is disabled
     */
    if (!strict)
        return HB_TYPE_ITEM;

    hb_errRT_BASE_SubstR(EG_DATATYPE, 50081, "could not get typecode from object", "hbgobject", HB_ERR_ARGS_BASEPARAMS);
    return 0;
}

/**
 * hbg_type_from_object:
 * obj: a Harbour item
 *
 * converts a Harbour item to a GType.  Raises an exception if it
 * can't perform the conversion.
 *
 * Returns: the corresponding GType, or 0 on error.
 */
GType
hbg_type_from_object(PHB_ITEM obj)
{
    /* Legacy call always defaults to strict type checking */
    return hbg_type_from_object_strict(obj, TRUE);
}

/* -------------- GValue marshalling ------------------ */

/**
 * hbg_enum_get_value:
 * @enum_type: the GType of the flag.
 * @obj: a Harbour object representing the flag value
 * @val: a pointer to the location to store the integer representation of the flag.
 *
 * Converts a Harbour object to the integer equivalent.  The conversion
 * will depend on the type of the Harbour object.  If the object is an
 * integer, it is passed through directly.  If it is a string, it will
 * be treated as a full or short enum name as defined in the GType.
 *
 * Returns: 0 on success or -1 on failure
 */
gint
hbg_enum_get_value(GType enum_type, PHB_ITEM obj, gint *val)
{
    GEnumClass *eclass = NULL;
    gint res = -1;

    g_return_val_if_fail(val != NULL, -1);
    if (!obj) {
	*val = 0;
	res = 0;
    } else if (HB_IS_NUMINT(obj)) {
	*val = hb_itemGetNLL(obj);
	res = 0;

	/*if (PyObject_TypeCheck(obj, &PyGEnum_Type) && ((PyGEnum *) obj)->gtype != enum_type) {
	    g_warning("expected enumeration type %s, but got %s instead",
		      g_type_name(enum_type),
		      g_type_name(((PyGEnum *) obj)->gtype));
	}*/
    } else if (HB_IS_STRING(obj)) {
	GEnumValue *info;
	const char *str = hb_itemGetCPtr(obj);

	if (enum_type != G_TYPE_NONE)
	    eclass = G_ENUM_CLASS(g_type_class_ref(enum_type));
	else {
            hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40099, "could not convert string to enum because there is no GType associated to look up the value", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
	    res = -1;
	}
	info = g_enum_get_value_by_name(eclass, str);
	g_type_class_unref(eclass);

	if (!info)
	    info = g_enum_get_value_by_nick(eclass, str);
	if (info) {
	    *val = info->value;
	    res = 0;
	} else {
            hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40098, "could not convert string", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
	    res = -1;
	}
    } else {
        hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40097, "enum values must be strings or ints", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
	res = -1;
    }
    return res;
}

/**
 * hbg_flags_get_value:
 * @flag_type: the GType of the flag.
 * @obj: a Harbour object representing the flag value
 * @val: a pointer to the location to store the integer representation of the flag.
 *
 * Converts a Harbour object to the integer equivalent.  The conversion
 * will depend on the type of the Harbour object.  If the object is an
 * integer, it is passed through directly.  If it is a string, it will
 * be treated as a full or short flag name as defined in the GType.
 * If it is a tuple, then the items are treated as strings and ORed
 * together.
 *
 * Returns: 0 on success or -1 on failure
 */
gint
hbg_flags_get_value(GType flag_type, PHB_ITEM obj, gint *val)
{
    GFlagsClass *fclass = NULL;
    gint res = -1;

    g_return_val_if_fail(val != NULL, -1);
    if (!obj) {
	*val = 0;
	res = 0;
    } else if (HB_IS_NUMINT(obj)) {
	*val = hb_itemGetNLL(obj);
	res = 0;
    } else if (HB_IS_STRING(obj)) {
	GFlagsValue *info;
	const char *str = hb_itemGetCPtr(obj);

	if (flag_type != G_TYPE_NONE)
	    fclass = G_FLAGS_CLASS(g_type_class_ref(flag_type));
	else {
            hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40089, "could not convert string to flag because there is no GType associated to look up the value", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
	    res = -1;
	}
	info = g_flags_get_value_by_name(fclass, str);
	g_type_class_unref(fclass);

	if (!info)
	    info = g_flags_get_value_by_nick(fclass, str);
	if (info) {
	    *val = info->value;
	    res = 0;
	} else {
            hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40098, "could not convert string", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
	    res = -1;
	}
    } else if (HB_IS_ARRAY(obj)) {
	int i, len;

	len = hb_itemSize(obj);
	*val = 0;
	res = 0;

	if (flag_type != G_TYPE_NONE)
	    fclass = G_FLAGS_CLASS(g_type_class_ref(flag_type));
	else {
            hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40089, "could not convert string to flag because there is no GType associated to look up the value", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
	    res = -1;
	}

	for (i = 1; i <= len; i++) {
	    const char *str = hb_arrayGetCPtr(obj, i);
	    GFlagsValue *info = g_flags_get_value_by_name(fclass, str);

	    if (!info)
		info = g_flags_get_value_by_nick(fclass, str);
	    if (info) {
		*val |= info->value;
	    } else {
                hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40098, "could not convert string", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
		res = -1;
		break;
	    }
	}
	g_type_class_unref(fclass);
    } else {
        hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40087, "flag values must be strings, integers, or arrays of strings", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
	res = -1;
    }
    return res;
}

typedef struct {
    fromvaluefunc fromvalue;
    tovaluefunc tovalue;
} HbGTypeMarshal;
static GQuark hbg_type_marshal_key = 0;

static HbGTypeMarshal *
hbg_type_lookup(GType type)
{
    GType	ptype = type;
    HbGTypeMarshal	*tm = NULL;

    /* recursively lookup types */
    while (ptype) {
	if ((tm = g_type_get_qdata(ptype, hbg_type_marshal_key)) != NULL)
	    break;
	ptype = g_type_parent(ptype);
    }
    return tm;
}

#if 0
/**
 * pyg_register_gtype_custom:
 * @gtype: the GType for the new type
 * @from_func: a function to convert GValues to Python objects
 * @to_func: a function to convert Python objects to GValues
 *
 * In order to handle specific conversion of gboxed types or new
 * fundamental types, you may use this function to register conversion
 * handlers.
 */

void
pyg_register_gtype_custom(GType gtype,
			  fromvaluefunc from_func,
                          tovaluefunc to_func)
{
    PyGTypeMarshal *tm;

    if (!pyg_type_marshal_key)
        pyg_type_marshal_key = g_quark_from_static_string("PyGType::marshal");

    tm = g_new(PyGTypeMarshal, 1);
    tm->fromvalue = from_func;
    tm->tovalue = to_func;
    g_type_set_qdata(gtype, pyg_type_marshal_key, tm);
}

static int
pyg_value_array_from_pyobject(GValue *value,
			      PyObject *obj,
			      const GParamSpecValueArray *pspec)
{
    int len;
    GValueArray *value_array;
    int i;

    len = PySequence_Length(obj);
    if (len == -1) {
	PyErr_Clear();
	return -1;
    }

    if (pspec && pspec->fixed_n_elements > 0 && len != pspec->fixed_n_elements)
	return -1;

    value_array = g_value_array_new(len);

    for (i = 0; i < len; ++i) {
	PyObject *item = PySequence_GetItem(obj, i);
	GType type;
	GValue item_value = { 0, };
	int status;

	if (! item) {
	    PyErr_Clear();
	    g_value_array_free(value_array);
	    return -1;
	}

	if (pspec && pspec->element_spec)
	    type = G_PARAM_SPEC_VALUE_TYPE(pspec->element_spec);
	else if (item == Py_None)
	    type = G_TYPE_POINTER; /* store None as NULL */
	else {
	    type = pyg_type_from_object((PyObject*)Py_TYPE(item));
	    if (! type) {
		PyErr_Clear();
		g_value_array_free(value_array);
		Py_DECREF(item);
		return -1;
	    }
	}

	g_value_init(&item_value, type);
	status = (pspec && pspec->element_spec)
	    ? pyg_param_gvalue_from_pyobject(&item_value, item, pspec->element_spec)
	    : pyg_value_from_pyobject(&item_value, item);
	Py_DECREF(item);

	if (status == -1) {
	    g_value_array_free(value_array);
	    g_value_unset(&item_value);
	    return -1;
	}

	g_value_array_append(value_array, &item_value);
	g_value_unset(&item_value);
    }

    g_value_take_boxed(value, value_array);
    return 0;
}
#endif

/**
 * hbg_value_from_hbitem:
 * @value: the GValue object to store the converted value in.
 * @obj: the Harbour object to convert.
 *
 * This function converts a Harbour object and stores the result in a
 * GValue.  The GValue must be initialised in advance with
 * g_value_init().  If the Harbour object can't be converted to the
 * type of the GValue, then an error is returned.
 *
 * Returns: 0 on success, -1 on error.
 */
int
hbg_value_from_hbitem(GValue *value, PHB_ITEM obj)
{
    switch (G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(value))) {
    case G_TYPE_INTERFACE:
	/* we only handle interface types that have a GObject prereq */
	if (g_type_is_a(G_VALUE_TYPE(value), G_TYPE_OBJECT)) {
	    if (HB_IS_NIL(obj))
		g_value_set_object(value, NULL);
	    else {
                if (!hb_clsIsParent(hb_objGetClass(obj), "GOBJECT")) {
		    return -1;
		}
		if (!G_TYPE_CHECK_INSTANCE_TYPE(hbgobject_get(obj),
						G_VALUE_TYPE(value))) {
		    return -1;
		}
		g_value_set_object(value, hbgobject_get(obj));
	    }
	} else {
	    return -1;
	}
	break;
    case G_TYPE_CHAR:
        if (HB_IS_STRING(obj)) {
#ifndef GLIB_VERSION_2_32
	    g_value_set_char(value, hb_itemGetCPtr(obj)[0]);
#else
	    g_value_set_schar(value, hb_itemGetCPtr(obj)[0]);
#endif
	} else {
	    //PyErr_Clear();
	    return -1;
	}

	break;
    case G_TYPE_UCHAR:
        if (HB_IS_NUMINT(obj)) {
	    glong val;
	    val = hb_itemGetNL(obj);
	    if (val >= 0 && val <= 255)
	      g_value_set_uchar(value, (guchar)hb_itemGetNL (obj));
	    else
	      return -1;
	} else if (HB_IS_STRING(obj)) {
	    g_value_set_uchar(value, hb_itemGetCPtr(obj)[0]);
	} else {
	    //PyErr_Clear();
	    return -1;
	}
	break;
    case G_TYPE_BOOLEAN:
	g_value_set_boolean(value, hb_itemGetL(obj));
	break;
    case G_TYPE_INT:
	g_value_set_int(value, hb_itemGetNL(obj));
	break;
    case G_TYPE_UINT:
	g_value_set_uint(value, hb_itemGetNLL(obj));
	break;
    case G_TYPE_LONG:
	g_value_set_long(value, hb_itemGetNLL(obj));
	break;
    case G_TYPE_ULONG:
        if (HB_IS_NUMINT(obj))
	    g_value_set_ulong(value, hb_itemGetNLL(obj));
        else
            return -1;
        break;
    case G_TYPE_INT64:
	g_value_set_int64(value, hb_itemGetNLL(obj));
	break;
    case G_TYPE_UINT64:
        if (HB_IS_NUMINT(obj))
            g_value_set_uint64(value, hb_itemGetNLL(obj));
        else
            return -1;
	break;
    case G_TYPE_ENUM:
	{
	    gint val = 0;
	    if (hbg_enum_get_value(G_VALUE_TYPE(value), obj, &val) < 0) {
		//PyErr_Clear();
		return -1;
	    }
	    g_value_set_enum(value, val);
	}
	break;
    case G_TYPE_FLAGS:
	{
	    gint val = 0;
	    if (hbg_flags_get_value(G_VALUE_TYPE(value), obj, &val) < 0) {
		//PyErr_Clear();
		return -1;
	    }
	    g_value_set_flags(value, val);
	}
	break;
    case G_TYPE_FLOAT:
	g_value_set_float(value, hb_itemGetND(obj));
	break;
    case G_TYPE_DOUBLE:
	g_value_set_double(value, hb_itemGetND(obj));
	break;
    case G_TYPE_STRING:
	if (HB_IS_NIL(obj)) {
	    g_value_set_string(value, NULL);
	} else {
	    g_value_set_string(value, hb_itemGetCPtr(obj));
	}
	break;
    case G_TYPE_POINTER:
	if (HB_IS_NIL(obj))
	    g_value_set_pointer(value, NULL);
	else if (HB_IS_OBJECT(obj) && hb_clsIsParent(hb_objGetClass(obj), "GPOINTER") &&
		   G_VALUE_HOLDS(value, hbg_pointer_gtype(obj)))
	    g_value_set_pointer(value, hbg_pointer_get(obj, gpointer));
	else if (HB_IS_POINTER(obj))
	    g_value_set_pointer(value, hb_itemGetPtr(obj));
	else
	    return -1;
	break;
    case G_TYPE_BOXED: {
        hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40000, "boxed not supported yet", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
#if 0
	PyGTypeMarshal *bm;

	if (obj == Py_None)
	    g_value_set_boxed(value, NULL);
	else if (G_VALUE_HOLDS(value, PY_TYPE_OBJECT))
	    g_value_set_boxed(value, obj);
	else if (PyObject_TypeCheck(obj, &PyGBoxed_Type) &&
		   G_VALUE_HOLDS(value, ((PyGBoxed *)obj)->gtype))
	    g_value_set_boxed(value, pyg_boxed_get(obj, gpointer));
        else if (G_VALUE_HOLDS(value, G_TYPE_VALUE)) {
            GType type;
            GValue *n_value;

            type = pyg_type_from_object((PyObject*)Py_TYPE(obj));
            if (G_UNLIKELY (! type)) {
                PyErr_Clear();
                return -1;
            }
            n_value = g_new0 (GValue, 1);
            g_value_init (n_value, type);
            g_value_take_boxed (value, n_value);
            return pyg_value_from_pyobject (n_value, obj);
        }
        else if (PySequence_Check(obj) &&
		   G_VALUE_HOLDS(value, G_TYPE_VALUE_ARRAY))
	    return pyg_value_array_from_pyobject(value, obj, NULL);
	else if (PYGLIB_PyUnicode_Check(obj) &&
                 G_VALUE_HOLDS(value, G_TYPE_GSTRING)) {
            GString *string;
            char *buffer;
            Py_ssize_t len;
            if (PYGLIB_PyUnicode_AsStringAndSize(obj, &buffer, &len))
                return -1;
            string = g_string_new_len(buffer, len);
	    g_value_set_boxed(value, string);
	    g_string_free (string, TRUE);
            break;
        }
	else if ((bm = pyg_type_lookup(G_VALUE_TYPE(value))) != NULL)
	    return bm->tovalue(value, obj);
	else if (PYGLIB_CPointer_Check(obj))
	    g_value_set_boxed(value, PYGLIB_CPointer_GetPointer(obj, NULL));
	else
	    return -1;
#endif
	break;
    }
    case G_TYPE_PARAM:
        hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40000, "GParam not supported yet", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
#if 0
	if (PyGParamSpec_Check(obj))
	    g_value_set_param(value, PYGLIB_CPointer_GetPointer(obj, NULL));
	else
	    return -1;
#endif
	break;
    case G_TYPE_OBJECT:
	if (HB_IS_NIL(obj)) {
	    g_value_set_object(value, NULL);
	} else if (HB_IS_OBJECT(obj) && hb_clsIsParent(hb_objGetClass(obj), "GOBJECT") &&
		   G_TYPE_CHECK_INSTANCE_TYPE(hbgobject_get(obj),
					      G_VALUE_TYPE(value))) {
	    g_value_set_object(value, hbgobject_get(obj));
	} else
	    return -1;
	break;
    default:
	{
	    HbGTypeMarshal *bm;
	    if ((bm = hbg_type_lookup(G_VALUE_TYPE(value))) != NULL)
		return bm->tovalue(value, obj);
	    break;
	}
    }
    /*if (PyErr_Occurred()) {
        g_value_unset(value);
        PyErr_Clear();
        return -1;
    }*/
    return 0;
}

/**
 * hbg_value_as_hbitem:
 * @value: the GValue object.
 * @copy_boxed: true if boxed values should be copied.
 *
 * This function creates/returns a Harbour wrapper object that
 * represents the GValue passed as an argument.
 *
 * Returns: a PHB_ITEM representing the value.
 */
PHB_ITEM
hbg_value_as_hbitem(const GValue *value, gboolean copy_boxed)
{
    gchar buf[128];

    switch (G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(value))) {
    case G_TYPE_INTERFACE:
	if (g_type_is_a(G_VALUE_TYPE(value), G_TYPE_OBJECT))
	    return hbgobject_new_sunk(g_value_get_object(value));
	else
	    break;
    case G_TYPE_CHAR: {
#ifndef GLIB_VERSION_2_32
	gint8 val = g_value_get_char(value);
#else
	gint8 val = g_value_get_schar(value);
#endif
	return hb_itemPutCL(NULL, (char *)&val, 1);
    }
    case G_TYPE_UCHAR: {
	guint8 val = g_value_get_uchar(value);
	return hb_itemPutCL(NULL, (char *)&val, 1);
    }
    case G_TYPE_BOOLEAN: {
	return hb_itemPutL(NULL, g_value_get_boolean(value));
    }
    case G_TYPE_INT:
	return hb_itemPutNL(NULL, g_value_get_int(value));
    case G_TYPE_UINT:
	{
	    /* in Harbour, the Int object is backed by a long.  If a
	       long can hold the whole value of an unsigned int, use
	       an Int.  Otherwise, use a Long object to avoid overflow.
	       This matches the ULongArg behavior in codegen/argtypes.h */
#if (G_MAXUINT <= G_MAXLONG)
	    return hb_itemPutNL(NULL, (glong) g_value_get_uint(value));
#else
	    return hb_itemPutNLL(NULL, (gulong) g_value_get_uint(value));
#endif
	}
    case G_TYPE_LONG:
	return hb_itemPutNLL(NULL, g_value_get_long(value));
    case G_TYPE_ULONG:
	{
	    gulong val = g_value_get_ulong(value);

            return hb_itemPutNLL(NULL, val);
	}
    case G_TYPE_INT64:
	{
	    gint64 val = g_value_get_int64(value);

            return hb_itemPutNLL(NULL, val);
	}
    case G_TYPE_UINT64:
	{
	    guint64 val = g_value_get_uint64(value);

            return hb_itemPutNLL(NULL, val);
	}
    case G_TYPE_ENUM:
	//return hbg_enum_from_gtype(G_VALUE_TYPE(value), g_value_get_enum(value));
	return hb_itemPutNLL(NULL, g_value_get_enum(value));
    case G_TYPE_FLAGS:
	//return hbg_flags_from_gtype(G_VALUE_TYPE(value), g_value_get_flags(value));
	return hb_itemPutNLL(NULL, g_value_get_flags(value));
    case G_TYPE_FLOAT:
	return hb_itemPutND(NULL, g_value_get_float(value));
    case G_TYPE_DOUBLE:
	return hb_itemPutND(NULL, g_value_get_double(value));
    case G_TYPE_STRING:
	{
	    const gchar *str = g_value_get_string(value);

	    if (str)
		return hb_itemPutC(NULL, str);
	    return hb_itemNew(NULL);
	}
    case G_TYPE_POINTER:
	/*return hbg_pointer_new(G_VALUE_TYPE(value),
			       g_value_get_pointer(value));*/
        return hb_itemPutPtr(NULL, g_value_get_pointer(value));
    case G_TYPE_BOXED: {
        hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40000, "boxed not supported yet", "hbgobject", HB_ERR_ARGS_BASEPARAMS );
#if 0
	PyGTypeMarshal *bm;

	if (G_VALUE_HOLDS(value, PY_TYPE_OBJECT)) {
	    PyObject *ret = (PyObject *)g_value_dup_boxed(value);
	    if (ret == NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	    }
	    return ret;
        } else if (G_VALUE_HOLDS(value, G_TYPE_VALUE)) {
            GValue *n_value = g_value_get_boxed (value);
            return pyg_value_as_pyobject(n_value, copy_boxed);
        } else if (G_VALUE_HOLDS(value, G_TYPE_VALUE_ARRAY)) {
	    GValueArray *array = (GValueArray *) g_value_get_boxed(value);
	    PyObject *ret = PyList_New(array->n_values);
	    int i;
	    for (i = 0; i < array->n_values; ++i)
		PyList_SET_ITEM(ret, i, pyg_value_as_pyobject
                                (array->values + i, copy_boxed));
	    return ret;
	} else if (G_VALUE_HOLDS(value, G_TYPE_GSTRING)) {
	    GString *string = (GString *) g_value_get_boxed(value);
	    PyObject *ret = PYGLIB_PyUnicode_FromStringAndSize(string->str, string->len);
	    return ret;
	}
	bm = pyg_type_lookup(G_VALUE_TYPE(value));
	if (bm) {
	    return bm->fromvalue(value);
	} else {
	    if (copy_boxed)
		return pyg_boxed_new(G_VALUE_TYPE(value),
				     g_value_get_boxed(value), TRUE, TRUE);
	    else
		return pyg_boxed_new(G_VALUE_TYPE(value),
				     g_value_get_boxed(value),FALSE,FALSE);
	}
#endif
    }
#if 0
    case G_TYPE_PARAM:
	return hbg_param_spec_new(g_value_get_param(value));
#endif
    case G_TYPE_OBJECT:
	return hbgobject_new_sunk(g_value_get_object(value));
    default:
	{
#if 0
	    PyGTypeMarshal *bm;
	    if ((bm = pyg_type_lookup(G_VALUE_TYPE(value))))
		return bm->fromvalue(value);
#endif
	    break;
	}
    }
    g_snprintf(buf, sizeof(buf), "unknown type %s",
	       g_type_name(G_VALUE_TYPE(value)));
    hb_errRT_BASE_SubstR( HBGOBJECT_ERR, 40001, buf, "hbgobject", HB_ERR_ARGS_BASEPARAMS );
    return NULL;
}

/* -------------- HbGClosure ----------------- */

static void
hbg_closure_invalidate(gpointer data, GClosure *closure)
{
    HbGClosure *pc = (HbGClosure *)closure;
    //PyGILState_STATE state;
    HB_SYMBOL_UNUSED(data);

    //state = pyglib_gil_state_ensure();
    hb_itemRelease(pc->callback);
    hb_itemRelease(pc->extra_args);
    hb_itemRelease(pc->swap_data);
    //pyglib_gil_state_release(state);

    pc->callback = NULL;
    pc->extra_args = NULL;
    pc->swap_data = NULL;
}

static void
hbg_closure_marshal(GClosure *closure,
		    GValue *return_value,
		    guint n_param_values,
		    const GValue *param_values,
		    gpointer invocation_hint,
		    gpointer marshal_data)
{
    //PyGILState_STATE state;
    HbGClosure *pc = (HbGClosure *)closure;
    guint i;
    HB_SYMBOL_UNUSED(invocation_hint);
    HB_SYMBOL_UNUSED(marshal_data);

    //state = pyglib_gil_state_ensure();

    if (HB_IS_SYMBOL(pc->callback)) {
        hb_vmPushSymbol(pc->callback);
        hb_vmPushNil();
    } else {
        hb_vmPushEvalSym();
        hb_vmPush(pc->callback);
    }

    for (i = 0; i < n_param_values; i++) {
	/* swap in a different initial data for connect_object() */
	if (i == 0 && G_CCLOSURE_SWAP_DATA(closure)) {
	    g_return_if_fail(pc->swap_data != NULL);
            hb_vmPush(pc->swap_data);
	} else {
	    PHB_ITEM item = hbg_value_as_hbitem(&param_values[i], FALSE);

	    /* error condition */
	    if (!item) {
                return;
		//goto out;
	    }
            hb_vmPush(item);
	}
    }
    /* params passed to function may have extra arguments */
    if (pc->extra_args) {
        for (i = 0; i < hb_itemSize(pc->extra_args); i++) {
            hb_vmPush(hb_arrayGetItemPtr(pc->extra_args, i + 1));
        }
    }
    if (HB_IS_SYMBOL(pc->callback)) {
        hb_vmProc(n_param_values + hb_itemSize(pc->extra_args));
    } else {
        hb_vmEval(n_param_values + hb_itemSize(pc->extra_args));
    }
    /*if (ret == NULL) {
	if (pc->exception_handler)
	    pc->exception_handler(return_value, n_param_values, param_values);
	else
	    PyErr_Print();
	goto out;
    }*/

    if (return_value && hbg_value_from_hbitem(return_value, hb_stackReturnItem()) != 0) {
        hb_errRT_BASE_SubstR(EG_DATATYPE, 50201, "can't convert return value to desired type", "hbgobject", HB_ERR_ARGS_BASEPARAMS);

	/*if (pc->exception_handler)
	    pc->exception_handler(return_value, n_param_values, param_values);
	else
	    PyErr_Print();*/
    }

 //out:
    //pyglib_gil_state_release(state);
}

/**
 * hbg_closure_new:
 * callback: a Harbour callable object
 * extra_args: a tuple of extra arguments, or None/NULL.
 * swap_data: an alternative Harbour object to pass first.
 *
 * Creates a GClosure wrapping a Harbour callable and optionally a set
 * of additional function arguments.  This is needed to attach Harbour
 * handlers to signals, for instance.
 *
 * Returns: the new closure.
 */
GClosure *
hbg_closure_new(PHB_ITEM callback, PHB_ITEM extra_args, PHB_ITEM swap_data)
{
    GClosure *closure;

    g_return_val_if_fail(callback != NULL, NULL);
    closure = g_closure_new_simple(sizeof(HbGClosure), NULL);
    g_closure_add_invalidate_notifier(closure, NULL, hbg_closure_invalidate);
    g_closure_set_marshal(closure, hbg_closure_marshal);
    ((HbGClosure *)closure)->callback = hb_itemNew(callback);
    if (extra_args && !HB_IS_NIL(extra_args)) {
	/*if (!PyTuple_Check(extra_args)) {
	    PyObject *tmp = PyTuple_New(1);
	    PyTuple_SetItem(tmp, 0, extra_args);
	    extra_args = tmp;
	}*/
	((HbGClosure *)closure)->extra_args = hb_itemNew(extra_args);
    }
    if (swap_data) {
	((HbGClosure *)closure)->swap_data = hb_itemNew(swap_data);
	closure->derivative_flag = TRUE;
    }
    return closure;
}

#if 0
/**
 * pyg_closure_set_exception_handler:
 * @closure: a closure created with pyg_closure_new()
 * @handler: the handler to call when an exception occurs or NULL for none
 *
 * Sets the handler to call when an exception occurs during closure invocation.
 * The handler is responsible for providing a proper return value to the
 * closure invocation. If @handler is %NULL, the default handler will be used.
 * The default handler prints the exception to stderr and doesn't touch the
 * closure's return value.
 */
void
pyg_closure_set_exception_handler(GClosure *closure,
				  PyClosureExceptionHandler handler)
{
    PyGClosure *pygclosure;

    g_return_if_fail(closure != NULL);

    pygclosure = (PyGClosure *)closure;
    pygclosure->exception_handler = handler;
}
/* -------------- PySignalClassClosure ----------------- */
/* a closure used for the `class closure' of a signal.  As this gets
 * all the info from the first argument to the closure and the
 * invocation hint, we can have a single closure that handles all
 * class closure cases.  We call a method by the name of the signal
 * with "do_" prepended.
 *
 *  We also remove the first argument from the * param list, as it is
 *  the instance object, which is passed * implicitly to the method
 *  object. */

static void
pyg_signal_class_closure_marshal(GClosure *closure,
				 GValue *return_value,
				 guint n_param_values,
				 const GValue *param_values,
				 gpointer invocation_hint,
				 gpointer marshal_data)
{
    PyGILState_STATE state;
    GObject *object;
    PyObject *object_wrapper;
    GSignalInvocationHint *hint = (GSignalInvocationHint *)invocation_hint;
    gchar *method_name, *tmp;
    PyObject *method;
    PyObject *params, *ret;
    guint i, len;

    state = pyglib_gil_state_ensure();

    g_return_if_fail(invocation_hint != NULL);
    /* get the object passed as the first argument to the closure */
    object = g_value_get_object(&param_values[0]);
    g_return_if_fail(object != NULL && G_IS_OBJECT(object));

    /* get the wrapper for this object */
    object_wrapper = pygobject_new_sunk(object);
    g_return_if_fail(object_wrapper != NULL);

    /* construct method name for this class closure */
    method_name = g_strconcat("do_", g_signal_name(hint->signal_id), NULL);

    /* convert dashes to underscores.  For some reason, g_signal_name
     * seems to convert all the underscores in the signal name to
       dashes??? */
    for (tmp = method_name; *tmp != '\0'; tmp++)
	if (*tmp == '-') *tmp = '_';

    method = PyObject_GetAttrString(object_wrapper, method_name);
    g_free(method_name);

    if (!method) {
	PyErr_Clear();
	Py_DECREF(object_wrapper);
	pyglib_gil_state_release(state);
	return;
    }
    Py_DECREF(object_wrapper);

    /* construct Python tuple for the parameter values; don't copy boxed values
       initially because we'll check after the call to see if a copy is needed. */
    params = PyTuple_New(n_param_values - 1);
    for (i = 1; i < n_param_values; i++) {
	PyObject *item = pyg_value_as_pyobject(&param_values[i], FALSE);

	/* error condition */
	if (!item) {
	    Py_DECREF(params);
	    pyglib_gil_state_release(state);
	    return;
	}
	PyTuple_SetItem(params, i - 1, item);
    }

    ret = PyObject_CallObject(method, params);

    /* Copy boxed values if others ref them, this needs to be done regardless of
       exception status. */
    len = PyTuple_Size(params);
    for (i = 0; i < len; i++) {
	PyObject *item = PyTuple_GetItem(params, i);
	if (item != NULL && PyObject_TypeCheck(item, &PyGBoxed_Type)
	    && item->ob_refcnt != 1) {
	    PyGBoxed* boxed_item = (PyGBoxed*)item;
	    if (!boxed_item->free_on_dealloc) {
		boxed_item->boxed = g_boxed_copy(boxed_item->gtype, boxed_item->boxed);
		boxed_item->free_on_dealloc = TRUE;
	    }
	}
    }

    if (ret == NULL) {
	PyErr_Print();
	Py_DECREF(method);
	Py_DECREF(params);
	pyglib_gil_state_release(state);
	return;
    }
    Py_DECREF(method);
    Py_DECREF(params);
    if (return_value)
	pyg_value_from_pyobject(return_value, ret);
    Py_DECREF(ret);
    pyglib_gil_state_release(state);
}

/**
 * pyg_signal_class_closure_get:
 *
 * Returns the GClosure used for the class closure of signals.  When
 * called, it will invoke the method do_signalname (for the signal
 * "signalname").
 *
 * Returns: the closure.
 */
GClosure *
pyg_signal_class_closure_get(void)
{
    static GClosure *closure;

    if (closure == NULL) {
	closure = g_closure_new_simple(sizeof(GClosure), NULL);
	g_closure_set_marshal(closure, pyg_signal_class_closure_marshal);

	g_closure_ref(closure);
	g_closure_sink(closure);
    }
    return closure;
}

GClosure *
gclosure_from_pyfunc(PyGObject *object, PyObject *func)
{
    GSList *l;
    PyGObjectData *inst_data;
    inst_data = pyg_object_peek_inst_data(object->obj);
    if (inst_data) {
        for (l = inst_data->closures; l; l = l->next) {
            PyGClosure *pyclosure = l->data;
            int res = PyObject_RichCompareBool(pyclosure->callback, func, Py_EQ);
            if (res == -1) {
                PyErr_Clear(); // Is there anything else to do?
            } else if (res) {
                return (GClosure*)pyclosure;
            }
        }
    }
    return NULL;
}

/* ----- __doc__ descriptor for GObject and GInterface ----- */

/* append information about signals of a particular gtype */
static void
add_signal_docs(GType gtype, GString *string)
{
    GTypeClass *class = NULL;
    guint *signal_ids, n_ids = 0, i;

    if (G_TYPE_IS_CLASSED(gtype))
	class = g_type_class_ref(gtype);
    signal_ids = g_signal_list_ids(gtype, &n_ids);

    if (n_ids > 0) {
	g_string_append_printf(string, "Signals from %s:\n",
			       g_type_name(gtype));

	for (i = 0; i < n_ids; i++) {
	    GSignalQuery query;
	    guint j;

	    g_signal_query(signal_ids[i], &query);

	    g_string_append(string, "  ");
	    g_string_append(string, query.signal_name);
	    g_string_append(string, " (");
	    for (j = 0; j < query.n_params; j++) {
		g_string_append(string, g_type_name(query.param_types[j]));
		if (j != query.n_params - 1)
		    g_string_append(string, ", ");
	    }
	    g_string_append(string, ")");
	    if (query.return_type && query.return_type != G_TYPE_NONE) {
		g_string_append(string, " -> ");
		g_string_append(string, g_type_name(query.return_type));
	    }
	    g_string_append(string, "\n");
	}
	g_free(signal_ids);
	g_string_append(string, "\n");
    }
    if (class)
	g_type_class_unref(class);
}

static void
add_property_docs(GType gtype, GString *string)
{
    GObjectClass *class;
    GParamSpec **props;
    guint n_props = 0, i;
    gboolean has_prop = FALSE;
    G_CONST_RETURN gchar *blurb=NULL;

    class = g_type_class_ref(gtype);
    props = g_object_class_list_properties(class, &n_props);

    for (i = 0; i < n_props; i++) {
	if (props[i]->owner_type != gtype)
	    continue; /* these are from a parent type */

	/* print out the heading first */
	if (!has_prop) {
	    g_string_append_printf(string, "Properties from %s:\n",
				   g_type_name(gtype));
	    has_prop = TRUE;
	}
	g_string_append_printf(string, "  %s -> %s: %s\n",
			       g_param_spec_get_name(props[i]),
			       g_type_name(props[i]->value_type),
			       g_param_spec_get_nick(props[i]));

	/* g_string_append_printf crashes on win32 if the third
	   argument is NULL. */
	blurb=g_param_spec_get_blurb(props[i]);
	if (blurb)
	    g_string_append_printf(string, "    %s\n",blurb);
    }
    g_free(props);
    if (has_prop)
	g_string_append(string, "\n");
    g_type_class_unref(class);
}

static PHB_ITEM
object_doc_descr_get(PyObject *self, PyObject *obj, PyObject *type)
{
    GType gtype = 0;
    GString *string;
    PyObject *pystring;

    if (obj && pygobject_check(obj, &PyGObject_Type)) {
	gtype = G_OBJECT_TYPE(pygobject_get(obj));
	if (!gtype)
	    PyErr_SetString(PyExc_RuntimeError, "could not get object type");
    } else {
	gtype = pyg_type_from_object(type);
    }
    if (!gtype)
	return NULL;

    string = g_string_new_len(NULL, 512);

    if (g_type_is_a(gtype, G_TYPE_INTERFACE))
	g_string_append_printf(string, "Interface %s\n\n", g_type_name(gtype));
    else if (g_type_is_a(gtype, G_TYPE_OBJECT))
	g_string_append_printf(string, "Object %s\n\n", g_type_name(gtype));
    else
	g_string_append_printf(string, "%s\n\n", g_type_name(gtype));

    if (((PyTypeObject *) type)->tp_doc)
        g_string_append_printf(string, "%s\n\n", ((PyTypeObject *) type)->tp_doc);

    if (g_type_is_a(gtype, G_TYPE_OBJECT)) {
	GType parent = G_TYPE_OBJECT;
        GArray *parents = g_array_new(FALSE, FALSE, sizeof(GType));
        int iparent;

        while (parent) {
            g_array_append_val(parents, parent);
            parent = g_type_next_base(gtype, parent);
        }

        for (iparent = parents->len - 1; iparent >= 0; --iparent) {
	    GType *interfaces;
	    guint n_interfaces, i;

            parent = g_array_index(parents, GType, iparent);
	    add_signal_docs(parent, string);
	    add_property_docs(parent, string);

	    /* add docs for implemented interfaces */
	    interfaces = g_type_interfaces(parent, &n_interfaces);
	    for (i = 0; i < n_interfaces; i++)
		add_signal_docs(interfaces[i], string);
	    g_free(interfaces);
	}
        g_array_free(parents, TRUE);
    }

    pystring = PYGLIB_PyUnicode_FromStringAndSize(string->str, string->len);
    g_string_free(string, TRUE);
    return pystring;
}

//PYGLIB_DEFINE_TYPE("gobject.GObject.__doc__", PyGObjectDoc_Type, PyObject);

/**
 * hbg_object_descr_doc_get:
 *
 * Returns an object intended to be the __doc__ attribute of GObject
 * wrappers.  When read in the context of the object it will return
 * some documentation about the signals and properties of the object.
 *
 * Returns: the descriptor.
 */
PHB_ITEM
hbg_object_descr_doc_get(void)
{
    static PHB_ITEM doc_descr = NULL;

    if (!doc_descr) {
	/*Py_TYPE(&PyGObjectDoc_Type) = &PyType_Type;
	if (PyType_Ready(&PyGObjectDoc_Type))
	    return NULL;*/

	doc_descr = hbgi_hb_clsInst(HbGObjectDoc_Type);
	if (doc_descr == NULL)
	    return NULL;
    }
    return doc_descr;
}


/**
 * pyg_pyobj_to_unichar_conv:
 *
 * Converts PyObject value to a unichar and write result to memory
 * pointed to by ptr.  Follows the calling convention of a ParseArgs
 * converter (O& format specifier) so it may be used to convert function
 * arguments.
 *
 * Returns: 1 if the conversion succeeds and 0 otherwise.  If the conversion
 *          did not succeesd, a Python exception is raised
 */
int pyg_pyobj_to_unichar_conv(PyObject* py_obj, void* ptr)
{
    gunichar* u = ptr;
    const Py_UNICODE* uni_buffer;
    PyObject* tmp_uni = NULL;

    if (PyUnicode_Check(py_obj)) {
	tmp_uni = py_obj;
	Py_INCREF(tmp_uni);
    }
    else {
	tmp_uni = PyUnicode_FromObject(py_obj);
	if (tmp_uni == NULL)
	    goto failure;
    }

    if ( PyUnicode_GetSize(tmp_uni) != 1) {
	PyErr_SetString(PyExc_ValueError, "unicode character value must be 1 character uniode string");
	goto failure;
    }
    uni_buffer = PyUnicode_AsUnicode(tmp_uni);
    if ( uni_buffer == NULL)
	goto failure;
    *u = uni_buffer[0];

    Py_DECREF(tmp_uni);
    return 1;

  failure:
    Py_XDECREF(tmp_uni);
    return 0;
}


int
pyg_param_gvalue_from_pyobject(GValue* value,
                               PyObject* py_obj,
			       const GParamSpec* pspec)
{
    if (G_IS_PARAM_SPEC_UNICHAR(pspec)) {
	gunichar u;

	if (!pyg_pyobj_to_unichar_conv(py_obj, &u)) {
	    PyErr_Clear();
	    return -1;
	}
        g_value_set_uint(value, u);
	return 0;
    }
    else if (G_IS_PARAM_SPEC_VALUE_ARRAY(pspec))
	return pyg_value_array_from_pyobject(value, py_obj,
					     G_PARAM_SPEC_VALUE_ARRAY(pspec));
    else {
	return pyg_value_from_pyobject(value, py_obj);
    }
}
#endif

PHB_ITEM
hbg_param_gvalue_as_hbitem(const GValue* gvalue,
                             gboolean copy_boxed,
			     const GParamSpec* pspec)
{
    if (G_IS_PARAM_SPEC_UNICHAR(pspec)) {
	gunichar u;
        HB_SIZE len;
	char uni_buffer[6] = "";

	u = g_value_get_uint(gvalue);
        len = g_unichar_to_utf8(u, uni_buffer);
	return hb_itemPutCL(NULL, uni_buffer, len);
    }
    else {
	return hbg_value_as_hbitem(gvalue, copy_boxed);
    }
}

#if 0
/**
 * pyg_type_registration_callback
 * @gtypename: type name
 * @callback: function to run
 *
 */
typedef struct {
    PyGTypeRegistrationFunction callback;
    gpointer data;
} CustomTypeData;

void
pyg_type_register_custom_callback(const gchar *typename,
				  PyGTypeRegistrationFunction callback,
				  gpointer user_data)
{
    CustomTypeData *data;

    if (!custom_type_registration)
	custom_type_registration = g_hash_table_new_full (g_str_hash, g_str_equal,
							  g_free, g_free);

    data = g_new (CustomTypeData, 1);
    data->callback = callback;
    data->data = user_data;

    g_hash_table_insert(custom_type_registration,
			g_strdup(typename),
			data);
}
#endif

HB_USHORT
hbg_type_get_custom(const gchar *name)
{
   HB_SYMBOL_UNUSED(name);
   return 0;
#if 0
    CustomTypeData *data;
    HB_USHORT retval;

    if (!custom_type_registration)
	return NULL;

    data = g_hash_table_lookup(custom_type_registration, name);
    if (!data)
	return NULL;

    retval = data->callback(name, data->data);

    g_hash_table_remove(custom_type_registration, name);

    return retval;
#endif
}

GType
_hbg_type_from_name(const gchar *name)
{
    GType type;

    type = g_type_from_name(name);
    if (type == G_TYPE_INVALID) {
	hbg_type_get_custom(name);
	type = g_type_from_name(name);
    }

    return type;
}

#if 0
static PyObject *
_pyg_strv_from_gvalue(const GValue *value)
{
    gchar    **argv = (gchar **) g_value_get_boxed(value);
    int        argc = 0, i;
    PyObject  *py_argv;

    if (argv) {
        while (argv[argc])
            argc++;
    }
    py_argv = PyList_New(argc);
    for (i = 0; i < argc; ++i)
	PyList_SET_ITEM(py_argv, i, PYGLIB_PyUnicode_FromString(argv[i]));
    return py_argv;
}

static int
_pyg_strv_to_gvalue(GValue *value, PyObject *obj)
{
    Py_ssize_t argc, i;
    gchar **argv;

    if (!(PyTuple_Check(obj) || PyList_Check(obj)))
        return -1;

    argc = PySequence_Length(obj);
    for (i = 0; i < argc; ++i)
	if (!PYGLIB_PyUnicode_Check(PySequence_Fast_GET_ITEM(obj, i)))
	    return -1;
    argv = g_new(gchar *, argc + 1);
    for (i = 0; i < argc; ++i)
	argv[i] = g_strdup(PYGLIB_PyUnicode_AsString(PySequence_Fast_GET_ITEM(obj, i)));
    argv[i] = NULL;
    g_value_take_boxed(value, argv);
    return 0;
}

void
pygobject_type_register_types(PyObject *d)
{
    PyGTypeWrapper_Type.tp_dealloc = (destructor)pyg_type_wrapper_dealloc;
    PyGTypeWrapper_Type.tp_richcompare = pyg_type_wrapper_richcompare;
    PyGTypeWrapper_Type.tp_repr = (reprfunc)pyg_type_wrapper_repr;
    PyGTypeWrapper_Type.tp_hash = (hashfunc)pyg_type_wrapper_hash;
    PyGTypeWrapper_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    PyGTypeWrapper_Type.tp_methods = _PyGTypeWrapper_methods;
    PyGTypeWrapper_Type.tp_getset = _PyGTypeWrapper_getsets;
    PyGTypeWrapper_Type.tp_init = (initproc)pyg_type_wrapper_init;
    PYGLIB_REGISTER_TYPE(d, PyGTypeWrapper_Type, "GType");

    /* This type lazily registered in pyg_object_descr_doc_get */
    PyGObjectDoc_Type.tp_dealloc = (destructor)object_doc_dealloc;
    PyGObjectDoc_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    PyGObjectDoc_Type.tp_descr_get = (descrgetfunc)object_doc_descr_get;

    pyg_register_gtype_custom(G_TYPE_STRV,
			      _pyg_strv_from_gvalue,
			      _pyg_strv_to_gvalue);
}
#endif
