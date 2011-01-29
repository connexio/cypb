/* 
 * Protocol Buffers decoder for Python
 *
 * (C) Copyright 2011, connex.io gmbh
 * */

#include<stdio.h>
#include<string.h>
#include<Python.h>

#define TYPE_DOUBLE         1
#define TYPE_FLOAT          2
#define TYPE_INT64          3
#define TYPE_UINT64         4
#define TYPE_INT32          5
#define TYPE_FIXED64        6
#define TYPE_FIXED32        7
#define TYPE_BOOL           8
#define TYPE_STRING         9
#define TYPE_GROUP          10
#define TYPE_MESSAGE        11
#define TYPE_BYTES          12
#define TYPE_UINT32         13
#define TYPE_ENUM           14
#define TYPE_SFIXED32       15
#define TYPE_SFIXED64       16
#define TYPE_SINT32         17
#define TYPE_SINT64         18

/* 
 * message types / fields descriptor structure
 * It is initialized by setup_table function, automatically generated code 
 * based on proto descriptor file.
 *
 * ft[Message_Type_ID][Field_Number]
 * Message Type ID is automatically assigned integer number to each message type.
 */
struct FieldDescriptor
{
    PyObject *name_str;         // pre-created PyString object for key / field name
    int repeated;
    int type;
    PyObject *default_val;
} ft[256][256];

int flist[256][256];

/* import setup_table function */
#include "descriptor_compiled.h"


/* Given message type and field number, returns type of field */
int get_type(int msgtype, int fieldid)
{
    if( ft[msgtype][fieldid].name_str ) {
        return ft[msgtype][fieldid].type;
    }
    return -1;
}

/* create new object of specific type and return pointer to it */
PyObject *object_new(int type)
{
    PyObject *obj = PyDict_New();

    int i;
    for(i=0; flist[type][i] != -1; i++) {
        struct FieldDescriptor *f = &ft[type][flist[type][i]];
        if( f->repeated )
            PyDict_SetItem(obj, f->name_str, PyList_New(0));
        // TODO: set default value for field.
    }
    return obj;
}

/* set attribute of object or append to list */
void object_add_field(PyObject* obj, int type, int field_id, PyObject* child)
{
    struct FieldDescriptor *f = &ft[type][field_id];
    if(f==NULL)
        return;

    if( ! f->repeated ) {
        PyDict_SetItem(obj, f->name_str, child);
    } else {
        PyObject* list;
        list = PyDict_GetItem(obj, f->name_str);
        PyList_Append(list, child);
    }
}

/* Varint, 32bit and 64bit decoders  */

char *get_varint(char *c, unsigned long *val)
{
    int shift = 0;
    *val = 0;
    do {
        *val |= (*c & 0x7F) << shift;
        if( (*c) & 0x80 ) 
            shift += 7;
        else {
            c++;
            break;
        }
        c++;
    } while(1);
    return c;
}

char *get_32bit(char *c, unsigned int *val)
{
    *val = c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24);
    c += 4;
    return c;
}

char *get_64bit(char *c, unsigned long *val)
{
    int i;
    *val = 0;
    for (i = 7; i >= 0; i--)
        *val = (*val << 8) | c[i];
    c += 8;
    return c;
}


/* Decoder context / stack. Used when decoding full message. */
struct context {
    PyObject *node;
    int type;
    int field_id;
    char *end_idx;
} stack[128];
int top = 0;

PyObject* full_decode(char *msg, int type)
{
    char *c = msg;
    // create root object
    top = 1;
    stack[top].node = object_new(type);
    stack[top].type = type;
    stack[top].end_idx = c + strlen(msg);

    while(*c) {

        unsigned long tag, len, val;
        unsigned int wire_type;
        int type = -1;

        c = get_varint(c, &tag);
        wire_type = tag & 0x07;
        tag >>= 3;

        type = get_type(stack[top].type, tag);

        switch(type) {
            case -1:
                // Unknown field.. Skip.
                // TODO: based on wire-type..
                break;

            case TYPE_STRING:
                c = get_varint(c, &len);

                // temporarily null-terminate string to slice it and restore back after copy.
                char tmp = c[len];
                c[len] = '\0';
                object_add_field(stack[top].node, stack[top].type, tag, PyString_FromString(c));
                c[len] = tmp;
                c = c + len;
                break;

            case TYPE_ENUM:
                c = get_varint(c, &val);
                object_add_field(stack[top].node, stack[top].type, tag, Py_BuildValue("i", val));
                break;

            case TYPE_INT32:
                c = get_varint(c, &val);
                object_add_field(stack[top].node, stack[top].type, tag, Py_BuildValue("i", val));
                break;

            default:
                // Treat it as type_message
                c = get_varint(c, &len);

                // create new node and push it to stack
                top++;
                stack[top].node = object_new(type);
                stack[top].type = type;
                stack[top].field_id = tag;
                stack[top].end_idx = c + len;
        }

        // Pop decoded messages from stack
        while( top && c >= stack[top].end_idx ) {
            if( top-1 ) {
                /* Link child node to parent node */
                object_add_field(stack[top-1].node, stack[top-1].type, stack[top].field_id, stack[top].node);
                top--;
            } else {
                return stack[top].node;
            }
        }
    }
    return NULL;
}

/* Python wrapper for full decode function */
static PyObject* py_decode(PyObject *self, PyObject *args)
{
    int type;
    char *msg;

    if (PyArg_ParseTuple(args, "is", &type, &msg)) {
         return full_decode(msg, type);
    }

    return NULL;
}

static PyMethodDef methods[] = {
    {"decode",  py_decode, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL} 
};


/* Lazy decoder */

typedef struct {
  PyObject_HEAD

  int decoded;
  int msg_type;
  char *msg_data;
  PyObject *fields;
} PBMsg;

static PBMsg* PBMsg_new(int msg_type, char *msg_data);
static PyObject* PBMsg_getattr(PyObject  *o, PyObject  *attr_name);

static int PBMsg_init(PBMsg *self, PyObject *arg, PyObject *kwds) { return 0; }

static void PBMsg_dealloc(PBMsg *self)  {
    free(self->msg_data);
    self->ob_type->tp_free((PyObject*)self); 
}

static PyObject *
PBMsg_new_py(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    int msg_type;
    char *msg_data;

    if (PyArg_ParseTuple(args, "is", &msg_type, &msg_data)) {
        return (PyObject *)PBMsg_new(msg_type, msg_data);
    }
}


PyTypeObject PBMsgType = {
  PyObject_HEAD_INIT(NULL)
  0,                            /*ob_size*/
  "ctest.PBMsg",                /*tp_name*/
  sizeof(PBMsg),                /*tp_basicsize*/
  0,                            /*tp_itemsize*/
  /* methods */
  (destructor)PBMsg_dealloc,    /*tp_dealloc*/
  0,                            /*tp_print*/
  0,                            /*tp_getattr*/
  0,                            /*tp_setattr*/
  0,                            /*tp_compare*/
  0,                            /*tp_repr*/
  0,                            /*tp_as_number*/
  0,                            /*tp_as_sequence*/
  0,                            /*tp_as_mapping*/
  0,                            /*tp_hash*/
  0,                            /*tp_call*/
  0,                            /*tp_str*/
  PBMsg_getattr,                /*tp_getattro*/
  0,                            /*tp_setattro*/
  0,                            /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  0,                            /*tp_doc*/
  0,                            /*tp_traverse*/
  0,                            /*tp_clear*/
  0,                            /*tp_richcompare*/
  0,                            /*tp_weaklistoffset*/
  0,                            /*tp_iter*/
  0,                            /*tp_iternext*/
  0,                            /*tp_methods*/
  0,                            /*tp_members*/
  0,                            /*tp_getset*/
  0,                            /*tp_base*/
  0,                            /*tp_dict*/
  0,                            /*tp_descr_get*/
  0,                            /*tp_descr_set*/
  0,                            /*tp_dictoffset*/
  (initproc)PBMsg_init,         /*tp_init*/
  0,                            /*tp_alloc*/
  PBMsg_new_py,                 /*tp_new*/
  0,                            /*tp_free*/
  0,                            /*tp_is_gc*/
};

/* Initialize new lazy decoder object */
static PBMsg* PBMsg_new(int msg_type, char *msg_data)
{
        PBMsg *self;
        self = (PBMsg *)PBMsgType.tp_alloc(&PBMsgType, 0);
        if (self == NULL) 
            return NULL;

        self->decoded = 0;
        self->msg_type = msg_type;

        /* take copy of buffer */
        self->msg_data = malloc(strlen(msg_data)+1);
        strcpy(self->msg_data, msg_data);

        return self;
}

/* Decode all primitive fields in current level, don't go deeper */
void PBMsg_decode(PBMsg *node)
{
    char *c = node->msg_data;
    node->fields = object_new(node->msg_type);
    
    while(*c) {

        char tmp;
        unsigned long tag, len, val;
        unsigned int wire_type;
        int type = -1;

        c = get_varint(c, &tag);
        wire_type = tag & 0x07;
        tag >>= 3;

        type = get_type(node->msg_type, tag);

        switch(type) {
            case -1:
                // Unknown field.. Skip.
                // TODO: based on wire-type..
                break;

            case TYPE_STRING:
                c = get_varint(c, &len);
                // temporarily null-terminate string to slice it and restore back after copy.
                tmp = c[len];
                c[len] = '\0';
                object_add_field(node->fields, node->msg_type, tag, PyString_FromString(c));
                c[len] = tmp;
                c = c + len;
                break;

            case TYPE_ENUM:
                c = get_varint(c, &val);
                object_add_field(node->fields, node->msg_type, tag, Py_BuildValue("i", val));
                break;

            case TYPE_INT32:
                c = get_varint(c, &val);
                object_add_field(node->fields, node->msg_type, tag, Py_BuildValue("i", val));
                break;

            default:
                c = get_varint(c, &len);
                // Create unparsed child node..
                tmp = c[len];
                c[len] = '\0';
                PBMsg *child = PBMsg_new(type, c);
                c[len] = tmp;

                object_add_field(node->fields, node->msg_type, tag, child);

                c += len;
        }
    }
 
    node->decoded = 1;
}

/* decode current node only when child attribute is accessed, then cache the result */
static PyObject* PBMsg_getattr(PyObject  *o, PyObject  *attr_name)
{
    PBMsg *self = (PBMsg*)o;

    if(!self->decoded)
        PBMsg_decode(self);

    return PyDict_GetItem(self->fields, attr_name);
}


PyMODINIT_FUNC initctest(void); /*proto*/
PyMODINIT_FUNC initctest(void)
{

    PBMsgType.tp_base = &PyBaseObject_Type;
    if (PyType_Ready(&PBMsgType) < 0)
        return;

    PyObject *mod = Py_InitModule("ctest", methods);

    Py_INCREF(&PBMsgType);
    PyModule_AddObject(mod, "PBMsg", (PyObject*)&PBMsgType);

    memset(ft, 0, sizeof(ft));
    setup_table();

    return;
}


