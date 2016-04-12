#ifdef _DEBUG
#undef _DEBUG 
#include <Python.h>
#include <structmember.h>
#define _DEBUG
#else if
#include <Python.h>
#include <structmember.h>
#endif

#include <stdint.h>

#include <sstream>

extern "C"{

	struct ExtType : public PyObject{
		PyObject * code;
		PyObject * data;
	};

	PyObject * PyExtType_new(struct _typeobject * typeobj, PyObject *, PyObject *){
		ExtType * self = (ExtType*)typeobj->tp_alloc(typeobj, sizeof(ExtType));

		self->code = 0;
		self->data = 0;

		return self;
	}

	static int PyExtType_Init(PyObject * self, PyObject * argv, PyObject * kw)
	{
		static char *kwlist[] = { "code", "data", NULL };

		if (!PyArg_ParseTupleAndKeywords(argv, kw, "|OO", kwlist, &((ExtType*)self)->code, &((ExtType*)self)->data)){
			return -1;
		}

		Py_INCREF(((ExtType*)self)->code);
		Py_INCREF(((ExtType*)self)->data);

		return 0;
	}

	static void PyExtType_Destruct(PyObject* self){
		Py_XDECREF(((ExtType*)self)->code);
		Py_XDECREF(((ExtType*)self)->data);

		self->ob_type->tp_free((PyObject*)self);
	}

	static PyObject* PyExtType_Str(PyObject* self)
	{
		std::ostringstream osstr;
		osstr << "ExtType(code=" << PyInt_AsLong(((ExtType*)self)->code) << ", data=\'" << PyString_AsString(((ExtType*)self)->data) << "\')";
		std::string str = osstr.str();
		return Py_BuildValue("s", str.c_str());
	}

	static PyObject* PyExtType_Repr(PyObject* Self)
	{
		return PyExtType_Str(Self);
	}

	static PyMemberDef ExtTypeMembers[] =
	{
		{ "code", T_OBJECT_EX, offsetof(ExtType, code), 0, "code" },
		{ "data", T_OBJECT_EX, offsetof(ExtType, data), 0, "data" },
		{ NULL, NULL, NULL, 0, NULL },
	};

	static PyTypeObject ExtTypeClassInfo =
	{
		PyVarObject_HEAD_INIT(NULL, 0)						//PyObject_VAR_HEAD
		"_cmsgpack.ExtType",								//const char *tp_name; /* For printing, in format "<module>.<name>" */
		sizeof(ExtType),									//Py_ssize_t tp_basicsize, tp_itemsize; /* For allocation */
		0,
		PyExtType_Destruct,									//destructor tp_dealloc;
		0,													// printfunc tp_print;
		0,													// getattrfunc tp_getattr;
		0,													// setattrfunc tp_setattr;
		0,													// cmpfunc tp_compare;
		PyExtType_Repr,										// reprfunc tp_repr;
		0,													// PyNumberMethods *tp_as_number;
		0,													// PySequenceMethods *tp_as_sequence;
		0,													// PyMappingMethods *tp_as_mapping;
		0,													// hashfunc tp_hash;
		0,													// ternaryfunc tp_call;
		PyExtType_Str,										// reprfunc tp_str;
		0,													// getattrofunc tp_getattro;
		0,													// setattrofunc tp_setattro;		
		0,													// PyBufferProcs *tp_as_buffer;
		Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,			// long tp_flags;
		"_cmsgpack.ExtType --- Extensioned by C++!",		// const char *tp_doc; /* Documentation string */
		0,													// traverseproc tp_traverse;
		0,													// inquiry tp_clear;
		0,													// richcmpfunc tp_richcompare;
		0,													// Py_ssize_t tp_weaklistoffset;
		0,													// getiterfunc tp_iter;
		0,													// iternextfunc tp_iternext;
		0,													// struct PyMethodDef *tp_methods;
		ExtTypeMembers,										// struct PyMemberDef *tp_members;
		0,													// struct PyGetSetDef *tp_getset;
		0,													// struct _typeobject *tp_base;
		0,													// PyObject *tp_dict;
		0,													// descrgetfunc tp_descr_get;
		0,													// descrsetfunc tp_descr_set;
		0,													// Py_ssize_t tp_dictoffset;
		PyExtType_Init,										// initproc tp_init;
		0,													// allocfunc tp_alloc;
		PyExtType_new,										// newfunc tp_new;
	};

	inline void  pack_sub_obj(unsigned char * strbuf, int & count, PyObject * obj, PyObject * func){
		if (PyDict_Check(obj)){
			int len = PyDict_Size(obj);

			if (len <= 15){
				strbuf[count++] = 0x80 | len;
			}
			else if (len <= 65535){
				strbuf[count++] = 0xde;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}
			else if (len <= 4294967296){
				strbuf[count++] = 0xdf;
				strbuf[count++] = (len & 0xff000000) >> 24;
				strbuf[count++] = (len & 0xff0000) >> 16;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}

			Py_ssize_t index = 0;
			PyObject * key = 0;
			PyObject * value = 0;
			while (PyDict_Next(obj, &index, &key, &value)){
				pack_sub_obj(strbuf, count, key, func);
				pack_sub_obj(strbuf, count, value, func);
			}
		}
		else if (PyList_Check(obj) || PyTuple_Check(obj) || PySet_Check(obj)){
			int len = PyList_Size(obj);

			if (len <= 15){
				strbuf[count++] = 0x90 | len;
			}
			else if (len <= 65535){
				strbuf[count++] = 0xdc;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}
			else if (len <= 4294967296){
				strbuf[count++] = 0xdd;
				strbuf[count++] = (len & 0xff000000) >> 24;
				strbuf[count++] = (len & 0xff0000) >> 16;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}

			for (int i = 0; i < len; i++){
				pack_sub_obj(strbuf, count, PyList_GetItem(obj, i), func);
			}
		}
		else if (PyString_Check(obj)){
			int len = PyString_Size(obj);

			if (len <= 31){
				strbuf[count++] = 0xa0 | (((unsigned char)len) & 0xff);
			}
			else if (len <= 255){
				strbuf[count++] = 0xd9;
				strbuf[count++] = len;
			}
			else if (len <= 65535){
				strbuf[count++] = 0xda;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}
			else if (len <= 4294967296){
				strbuf[count++] = 0xdb;
				strbuf[count++] = (len & 0xff000000) >> 24;
				strbuf[count++] = (len & 0xff0000) >> 16;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}

			memcpy(&strbuf[count], PyString_AsString(obj), len);
			count += len;
		}
		else if (PyBytes_Check(obj)){
			int len = PyString_Size(obj);

			if (len <= 255){
				strbuf[count++] = 0xc4;
				strbuf[count++] = len;
			}
			else if (len <= 65535){
				strbuf[count++] = 0xc5;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}
			else if (len <= 4294967295){
				strbuf[count++] = 0xc6;
				strbuf[count++] = (len & 0xff000000) >> 24;
				strbuf[count++] = (len & 0xff0000) >> 16;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}

			memcpy(&strbuf[count], PyString_AsString(obj), len);
			count += len;
		}
		else if (PyUnicode_Check(obj)){
			PyObject * str = PyUnicode_AsASCIIString(obj);
			int len = PyBytes_Size(str);

			if (len <= 31){
				strbuf[count++] = 0xa0 | (((unsigned char)len) & 0xff);
			}
			else if (len <= 255){
				strbuf[count++] = 0xd9 | len;
				strbuf[count++] = len;
			}
			else if (len <= 65535){
				strbuf[count++] = 0xda;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}
			else if (len <= 4294967296){
				strbuf[count++] = 0xdb;
				strbuf[count++] = (len & 0xff000000) >> 24;
				strbuf[count++] = (len & 0xff0000) >> 16;
				strbuf[count++] = (len & 0xff00) >> 8;
				strbuf[count++] = len & 0xff;
			}

			memcpy(&strbuf[count], PyString_AsString(str), len);
			count += len;
		}
		else if (obj == Py_None){
			strbuf[count++] = 0xc0;
		}
		else if (PyBool_Check(obj)){
			strbuf[count++] = (PyInt_AsLong(obj) == 0) ? 0xc2 : 0xc3;
		}
		else if (PyFloat_Check(obj)){
			double v = PyFloat_AsDouble(obj);
			unsigned char * dbuf = (unsigned char*)&v;

			strbuf[count++] = 0xcb;
			strbuf[count++] = dbuf[7];
			strbuf[count++] = dbuf[6];
			strbuf[count++] = dbuf[5];
			strbuf[count++] = dbuf[4];
			strbuf[count++] = dbuf[3];
			strbuf[count++] = dbuf[2];
			strbuf[count++] = dbuf[1];
			strbuf[count++] = dbuf[0];
		}
		else if (PyLong_Check(obj)){
			int64_t v = PyLong_AsLongLong(obj);

			if (v < 0){
				if (v >= -32){
					strbuf[count++] = v;
				}
				else if (v >= -128){
					strbuf[count++] = 0xd0;
					strbuf[count++] = v;
				}
				else if (v >= -32768){
					short value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xd1;
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
				else if (v >= -2147483648){
					int value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xd2;
					strbuf[count++] = _vlaue[3];
					strbuf[count++] = _vlaue[2];
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
				else{
					unsigned char * _vlaue = (unsigned char *)&v;

					strbuf[count++] = 0xd3;
					strbuf[count++] = _vlaue[7];
					strbuf[count++] = _vlaue[6];
					strbuf[count++] = _vlaue[5];
					strbuf[count++] = _vlaue[4];
					strbuf[count++] = _vlaue[3];
					strbuf[count++] = _vlaue[2];
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
			}
			else if (v >= 0){
				if (v <= 127){
					strbuf[count++] = v;
				}
				else if (v <= 255){
					strbuf[count++] = 0xcc;
					strbuf[count++] = v;
				}
				else if (v <= 65535){
					unsigned short value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xcd;
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
				else if (v <= 4294967295){
					unsigned int value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xce;
					strbuf[count++] = _vlaue[3];
					strbuf[count++] = _vlaue[2];
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
				else {
					uint64_t value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xcf;
					strbuf[count++] = _vlaue[7];
					strbuf[count++] = _vlaue[6];
					strbuf[count++] = _vlaue[5];
					strbuf[count++] = _vlaue[4];
					strbuf[count++] = _vlaue[3];
					strbuf[count++] = _vlaue[2];
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
			}
		}
		else if (PyInt_Check(obj)){
			int64_t v = PyInt_AsSsize_t(obj);

			if (v < 0){
				if (v >= -32){
					strbuf[count++] = v;
				}
				else if (v >= -128){
					strbuf[count++] = 0xd0;
					strbuf[count++] = v;
				}
				else if (v >= -32768){
					short value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xd1;
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
				else if (v >= -2147483648){
					int value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xd2;
					strbuf[count++] = _vlaue[3];
					strbuf[count++] = _vlaue[2];
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
				else{
					unsigned char * _vlaue = (unsigned char *)&v;

					strbuf[count++] = 0xd3;
					strbuf[count++] = _vlaue[7];
					strbuf[count++] = _vlaue[6];
					strbuf[count++] = _vlaue[5];
					strbuf[count++] = _vlaue[4];
					strbuf[count++] = _vlaue[3];
					strbuf[count++] = _vlaue[2];
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
			}
			else if (v >= 0){
				if (v <= 127){
					strbuf[count++] = v;
				}
				else if (v <= 255){
					strbuf[count++] = 0xcc;
					strbuf[count++] = v;
				}
				else if (v <= 65535){
					unsigned short value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xcd;
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
				else if (v <= 4294967295){
					unsigned int value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xce;
					strbuf[count++] = _vlaue[3];
					strbuf[count++] = _vlaue[2];
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
				else {
					uint64_t value = v;
					unsigned char * _vlaue = (unsigned char *)&value;

					strbuf[count++] = 0xcf;
					strbuf[count++] = _vlaue[7];
					strbuf[count++] = _vlaue[6];
					strbuf[count++] = _vlaue[5];
					strbuf[count++] = _vlaue[4];
					strbuf[count++] = _vlaue[3];
					strbuf[count++] = _vlaue[2];
					strbuf[count++] = _vlaue[1];
					strbuf[count++] = _vlaue[0];
				}
			}
		}
		else if (Py_TYPE(obj) == &ExtTypeClassInfo){
			char code = PyInt_AsLong(((ExtType *)obj)->code);
			int len = PyString_Size(((ExtType *)obj)->data);

			if (len == 1){
				strbuf[count++] = 0xd4;
			}
			else if (len == 2){
				strbuf[count++] = 0xd5;
			}
			else if (len == 4){
				strbuf[count++] = 0xd6;
			}
			else if (len == 8){
				strbuf[count++] = 0xd7;
			}
			else if (len == 16){
				strbuf[count++] = 0xd8;
			}
			else if (len <= 0xff){
				strbuf[count++] = 0xc7;
				strbuf[count++] = len;
			}
			else if (len <= 0xffff) {
				unsigned char * lbuf = (unsigned char *)len;

				strbuf[count++] = 0xc8;
				strbuf[count++] = lbuf[1];
				strbuf[count++] = lbuf[0];
			}
			else{
				unsigned char * lbuf = (unsigned char *)len;

				strbuf[count++] = 0xc9;
				strbuf[count++] = lbuf[3];
				strbuf[count++] = lbuf[2];
				strbuf[count++] = lbuf[1];
				strbuf[count++] = lbuf[0];
			}

			int datasize = PyString_Size(((ExtType *)obj)->data);
			strbuf[count++] = code;
			memcpy(&strbuf[count], PyString_AsString(((ExtType *)obj)->data), datasize);
			count += datasize;
		}
		else {
			if (PyFunction_Check(func)){
				obj = PyObject_CallObject(func, PyTuple_Pack(1, obj));
				pack_sub_obj(strbuf, count, obj, func);
			}
		}
	}

	static PyObject * _pack(PyObject * self, PyObject * packer){
		PyObject * obj = PyTuple_GetItem(packer, 0);
		PyObject * func = PyTuple_GetItem(packer, 1);

		unsigned char strbuf[65536];
		int count = 0;
		pack_sub_obj(strbuf, count, obj, func);

		return PyString_FromStringAndSize((char*)strbuf, count);
	}

	inline int pack_to_python(PyObject ** obj, const char* data, size_t len, PyObject * func);

	inline int unpack_nil(PyObject ** obj, unsigned char code, const char * data, size_t len){
		if (code == 0xc0){
			*obj = Py_None;
		}

		return 0;
	}

	inline int unpack_bool(PyObject ** obj, unsigned char code, const char * data, size_t len){
		if (code == 0xc2){
			*obj = Py_False;
		}
		else if (code == 0xc3){
			*obj = Py_True;
		}

		return 0;
	}

	inline int unpack_binary(PyObject ** obj, unsigned char code, const char * data, size_t _len){
		int len = 0; int lensize = 0; const char * srcbuf = 0;
		if (code == 0xc4){
			if (_len < 1){
				return _len;
			}

			len = data[0];
			lensize = 1;
			srcbuf = &data[1];
		}
		else if (code == 0xc5){
			if (_len < 2){
				return _len;
			}

			len = data[0] << 8 | data[1];
			lensize = 2;
			srcbuf = &data[2];
		}
		else if (code == 0xc6){
			if (_len < 4){
				return _len;
			}

			len = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
			lensize = 4;
			srcbuf = &data[4];
		}

		if (_len < (len + lensize)){
			return _len;
		}

		*obj = PyString_FromStringAndSize(srcbuf, len);

		return len + lensize;
	}

	inline int unpack_raw(PyObject ** obj, unsigned char code, const char * data, size_t _len){
		int len = 0; int lensize = 0; const char * srcbuf = 0;
		if (code == 0xd9){
			if (_len < 1){
				return _len;
			}

			len = data[0];
			lensize = 1;
			srcbuf = &data[1];
		}
		else if (code == 0xda){
			if (_len < 2){
				return _len;
			}

			len = data[0] << 8 | data[1];
			lensize = 2;
			srcbuf = &data[2];
		}
		else if (code == 0xdb){
			if (_len < 4){
				return _len;
			}

			len = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
			lensize = 4;
			srcbuf = &data[4];
		}

		if (_len < (len + lensize)){
			return _len;
		}

		*obj = PyString_FromStringAndSize(srcbuf, len);

		return len + lensize;
	}

	inline int unpack_ext(PyObject ** obj, unsigned char code, const char * data, size_t _len, PyObject * func){
		int len = 0; int lensize = 0; char extcode = 0; const char * srcbuf = 0;
		if (code == 0xc7){
			if (_len < 1){
				return _len;
			}

			len = data[0];
			lensize = 1;
			extcode = data[1];
			srcbuf = &data[2];
		}
		else if (code == 0xc8){
			if (_len < 2){
				return _len;
			}

			len = data[0] << 8 | data[1];
			lensize = 2;
			extcode = data[2];
			srcbuf = &data[3];
		}
		else if (code == 0xc9){
			if (_len < 4){
				return _len;
			}

			len = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
			lensize = 4;
			extcode = data[4];
			srcbuf = &data[5];
		}

		if (_len < (len + lensize + 1)){
			return _len;
		}

		ExtType * self = (ExtType *)PyExtType_new(&ExtTypeClassInfo, 0, 0);
		self->code = PyInt_FromLong(extcode);
		self->data = PyString_FromStringAndSize(srcbuf, len);
		
		if (PyFunction_Check(func)){
			*obj = PyObject_CallObject(func, PyTuple_Pack(2, self->code, self->data));
		}
		else{
			*obj = self;
		}

		return len + lensize + 1;
	}

	inline int unpack_float(PyObject ** obj, unsigned char code, const char * data, size_t _len){
		if (code == 0xca){
			if (_len < 4){
				return _len;
			}

			char buf[4];
			buf[0] = data[3];
			buf[1] = data[2];
			buf[2] = data[1];
			buf[3] = data[0];
			*obj = PyFloat_FromDouble(*((float*)(buf)));

			return 4;
		}
		else if (code == 0xcb){
			if (_len < 8){
				return _len;
			}

			char buf[8];
			buf[0] = data[7];
			buf[1] = data[6];
			buf[2] = data[5];
			buf[3] = data[4];
			buf[4] = data[3];
			buf[5] = data[2];
			buf[6] = data[1];
			buf[7] = data[0];

			*obj = PyFloat_FromDouble(*((double*)(buf)));

			return 8;
		}

		return 0;
	}

	inline int unpack_uint64(PyObject ** obj, unsigned char code, const unsigned char * data, size_t _len){
		if (code == 0xcc){
			if (_len < 1){
				return _len;
			}
			*obj = PyLong_FromLongLong((uint64_t)data[0]);
			return 1;
		}
		else if (code == 0xcd){
			if (_len < 2){
				return _len;
			}
			unsigned short v = 0;
			unsigned char * _v = (unsigned char *)&v;
			_v[0] = data[1];
			_v[1] = data[0];
			*obj = PyLong_FromLongLong(v);
			return 2;
		}
		else if (code == 0xce){
			if (_len < 4){
				return _len;
			}
			unsigned int v = 0;
			unsigned char * _v = (unsigned char *)&v;
			_v[0] = data[3];
			_v[1] = data[2];
			_v[2] = data[1];
			_v[3] = data[0];

			*obj = PyLong_FromLongLong(v);
			return 4;
		}
		else if (code == 0xcf){
			if (_len < 8){
				return _len;
			}
			uint64_t v = 0;
			unsigned char * _v = (unsigned char *)&v;
			_v[0] = data[7];
			_v[1] = data[6];
			_v[2] = data[5];
			_v[3] = data[4];
			_v[4] = data[3];
			_v[5] = data[2];
			_v[6] = data[1];
			_v[7] = data[0];

			*obj = PyLong_FromLongLong(v);
			return 8;
		}

		return 0;
	}

	inline int unpack_int64(PyObject ** obj, unsigned char code, const unsigned char * data, size_t _len){
		if (code == 0xd0){
			if (_len < 1){
				return _len;
			}
			*obj = PyLong_FromLongLong(*((char*)&data[0]));
			return 1;
		}
		else if (code == 0xd1){
			if (_len < 2){
				return _len;
			}
			short v = 0;
			unsigned char * _v = (unsigned char *)&v;
			_v[0] = data[1];
			_v[1] = data[0];
			*obj = PyLong_FromLongLong(v);
			return 2;
		}
		else if (code == 0xd2){
			if (_len < 4){
				return _len;
			}
			int v = 0;
			unsigned char * _v = (unsigned char *)&v;
			_v[0] = data[3];
			_v[1] = data[2];
			_v[2] = data[1];
			_v[3] = data[0];

			*obj = PyLong_FromLongLong(v);
			return 4;
		}
		else if (code == 0xd3){
			if (_len < 8){
				return _len;
			}
			
			int64_t v = 0;
			unsigned char * _v = (unsigned char *)&v;
			_v[0] = data[7];
			_v[1] = data[6];
			_v[2] = data[5];
			_v[3] = data[4];
			_v[4] = data[3];
			_v[5] = data[2];
			_v[6] = data[1];
			_v[7] = data[0];

			*obj = PyLong_FromLongLong(v);
			return 8;
		}

		return 0;
	}

	inline int unpack_fixed_ext(PyObject ** obj, unsigned char code, const char * data, size_t _len, PyObject * func){
		int len = 0; char extcode = 0; const char * srcbuf = 0;
		if (code == 0xd4){
			len = 2;
		}
		else if (code == 0xd5){
			len = 3;
		}
		else if (code == 0xd6){
			len = 5;
		}
		else if (code == 0xd7){
			len = 9;
		}
		else if (code == 0xd8){
			len = 17;
		}

		if (_len < len){
			return _len;
		}

		extcode = data[0];
		srcbuf = &data[1];

		ExtType * self = (ExtType *)PyExtType_new(&ExtTypeClassInfo, 0, 0);
		self->code = PyInt_FromLong(extcode);
		self->data = PyString_FromStringAndSize(srcbuf, len);

		if (PyFunction_Check(func)){
			*obj = PyObject_CallObject(func, PyTuple_Pack(2, self->code, self->data));
		}
		else{
			*obj = self;
		}

		return len;
	}

	inline int unpack_array(PyObject ** obj, unsigned char code, const char * data, size_t _len, PyObject * func){
		int len = 0; int lensize = 0; int offset = 0;
		if (code == 0xdc){
			if (_len < 2){
				return _len;
			}

			offset = lensize = 2;
			len = (int)(data[0] << 8 | data[1]);
		}
		else if (code == 0xdd){
			if (_len < 4){
				return _len;
			}

			offset = lensize = 4;
			len = (int)(data[0] << 24 | data[1] << 16 | data[2] << 8 << data[3]);
		}

		*obj = PyList_New(len);
		for (int i = 0; i < len; i++){
			PyObject * subobj = 0;
			offset += pack_to_python(&subobj, &data[offset], _len - offset, func);
			PyList_SetItem(*obj, i, subobj);
		}

		return offset;
	}

	inline int unpack_map(PyObject ** obj, unsigned char code, const char * data, size_t _len, PyObject * func){
		int len = 0; int lensize = 0; int offset = 0;
		if (code == 0xde){
			if (_len < 2){
				return _len;
			}

			offset = lensize = 2;
			len = (int)(data[0] << 8 | data[1]);
		}
		else if (code == 0xdf){
			if (_len < 4){
				return _len;
			}

			offset = lensize = 4;
			len = (int)(data[0] << 24 | data[1] << 16 | data[2] << 8 << data[3]);
		}

		*obj = PyDict_New();
		for (int i = 0; i < len; i++){
			PyObject * key = 0, * value = 0;
			offset += pack_to_python(&key, &data[offset], _len - offset, func);
			offset += pack_to_python(&value, &data[offset], _len - offset, func);
			PyDict_SetItem(*obj, key, value);
		}

		return offset;
	}

	inline int unpack_fixed_array(PyObject ** obj, unsigned char code, const char * data, size_t _len, PyObject * func){
		int len = 0; int offset = 0;
		if ((code & 0xf0) == 0x90){
			len = code & ~0xf0;
		}

		*obj = PyList_New(len);
		for (int i = 0; i < len; i++){
			PyObject * subobj = 0;
			offset += pack_to_python(&subobj, &data[offset], _len - offset, func);
			PyList_SetItem(*obj, i, subobj);
		}

		return offset;
	}

	inline int unpack_fixed_map(PyObject ** obj, unsigned char code, const char * data, size_t _len, PyObject * func){
		int len = 0; int offset = 0;
		if ((code & 0xf0) == 0x80){
			len = code & ~0xf0;
		}

		*obj = PyDict_New();
		for (int i = 0; i < len; i++){
			PyObject * key = 0, *value = 0;
			offset += pack_to_python(&key, &data[offset], _len - offset, func);
			offset += pack_to_python(&value, &data[offset], _len - offset, func);
			PyDict_SetItem(*obj, key, value);
		}

		return offset;
	}

	inline int unpack_fixed_raw(PyObject ** obj, unsigned char code, const char * data, size_t _len){
		int len = 0; int offset = 0;
		if ((code & 0xe0) == 0xa0){
			len = code & ~0xe0;
		}

		if (_len < len){
			return _len;
		}

		*obj = PyString_FromStringAndSize(data, len);

		return len;
	}

	inline int pack_to_python(PyObject ** obj, const char* data, size_t len, PyObject * func){
		int offset = 0;

		if (len <= 0){
			return 0;
		}

		unsigned char code = data[0];
		if (code >= 0x00 && code <= 0x7f){
			*obj = PyInt_FromLong(code);
		}
		else if (code >= 0xe0 && code <= 0xff){
			*obj = PyInt_FromLong((int8_t)code);
		}
		else if (code >= 0xc0 && code <= 0xdf){
			switch (code)
			{
			case 0xc0:  // nil
				offset = unpack_nil(obj, code, 0, 0);
				break;
			case 0xc2:  // false
			case 0xc3:  // true
				offset = unpack_bool(obj, code, 0, 0);
				break;
			case 0xc4: // bin 8
			case 0xc5: // bin 16
			case 0xc6: // bin 32
				offset = unpack_binary(obj, code, &data[1], len - 1);
				break;
			case 0xc7:
			case 0xc8:
			case 0xc9:
				offset = unpack_ext(obj, code, &data[1], len - 1, func);
				break;
			case 0xca:  // float
			case 0xcb:  // double
				offset = unpack_float(obj, code, &data[1], len - 1);
				break;
			case 0xcc:  // unsigned int  8
			case 0xcd:  // unsigned int 16
			case 0xce:  // unsigned int 32
			case 0xcf:  // unsigned int 64
				offset = unpack_uint64(obj, code, (unsigned char*)&data[1], len - 1);
				break;
			case 0xd0:  // signed int  8
			case 0xd1:  // signed int 16
			case 0xd2:  // signed int 32
			case 0xd3:  // signed int 64
				offset = unpack_int64(obj, code, (unsigned char*)&data[1], len - 1);
				break;
			case 0xd4:
			case 0xd5:
			case 0xd6:  // big integer 16
			case 0xd7:  // big integer 32
			case 0xd8:  // big float 16
				offset = unpack_fixed_ext(obj, code, &data[1], len - 1, func);
				break;
			case 0xd9:  // raw 8 (str 8)
			case 0xda:  // raw 16 (str 16)
			case 0xdb:  // raw 32 (str 32)
				offset = unpack_raw(obj, code, &data[1], len - 1);
				break;
			case 0xdc:  // array 16
			case 0xdd:  // array 32
				offset = unpack_array(obj, code, &data[1], len - 1, func);
				break;
			case 0xde:  // map 16
			case 0xdf:  // map 32
				offset = unpack_map(obj, code, &data[1], len - 1, func);
				break;

			default:
				break;
			}
		}
		else if (code >= 0x90 && code <= 0x9f){
			offset = unpack_fixed_array(obj, code, &data[1], len - 1, func);
		}
		else if (code >= 0x80 && code <= 0x8f){
			offset = unpack_fixed_map(obj, code, &data[1], len - 1, func);
		}
		else if (code >= 0xa0 && code <= 0xbf){
			offset = unpack_fixed_raw(obj, code, &data[1], len - 1);
		}

		return offset + 1;
	}

	static PyObject * _unpack(PyObject * self, PyObject * packer){
		PyObject * obj = PyTuple_GetItem(packer, 0);
		PyObject * func = PyTuple_GetItem(packer, 1);

		PyObject * ret = 0;
		pack_to_python(&ret, PyString_AsString(obj), PyString_Size(obj), func);

		return ret;
	}

	static PyMethodDef msgpack_methods[] = {
			{ "_pack", _pack, METH_VARARGS },
			{ "_unpack", _unpack, METH_VARARGS },
			{ NULL, NULL }
	};

	PyMODINIT_FUNC init_cmsgpack(void)
	{
		PyObject * module = Py_InitModule("_cmsgpack", msgpack_methods);

		if (module != 0){
			if (PyType_Ready(&ExtTypeClassInfo) < 0){
				printf("ExtTypeClassInfo is unready\n");
			}

			Py_INCREF(&ExtTypeClassInfo);
			PyModule_AddObject(module, "ExtType", (PyObject*)&ExtTypeClassInfo);
		}
	}
}