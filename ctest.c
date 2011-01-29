
#include<stdio.h>
#include<string.h>
#include<Python.h>


char *get_tag(char *c, long *tag)
{
    int shift = 0;
    *tag = 0;
    do {
        *tag |= (*c & 0x7F) << shift;
        if( (*c) & 0x80 ) 
            shift += 7;
        else {
            c++;
            break;
        }
        c++;
    } while(1);
    *tag >>= 3;
    return c;
}

char *get_msg_len(char *c, long *len)
{
    int shift = 0;
    *len = 0;
    do {
        *len |= (*c & 0x7F) << shift;
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

/* 
 *  Types table
 *    primitives: 
 *       0: string
 *    messages:
 *       1: contact
 *       2: email
 *       3: phone
 *       4: name
*/

int get_type(int msgtype, int fieldid)
{
    // Contact
    if(msgtype==1) {
        switch(fieldid) {
            case 1:
                return 4;
            case 5:
                return 2;
            case 6:
                return 3;
            case 12:
                return 0;
        }
    }

    if(msgtype==2) {
        if(fieldid==1)
            return 0;
    }

    if(msgtype==3) {
        if(fieldid==1)
            return 0;
    }

    if(msgtype==4) {
        switch(fieldid) {
            case 1:
                return 0;
            case 3:
                return 0;
            case 4:
                return 0;
        }
    }

    return -1;
}

PyObject *pystr_emails, *pystr_phones, *pystr_note, *pystr_first, *pystr_last, *pystr_display_name, *pystr_email, *pystr_phone, *pystr_name;

/* create new object of specific type and return pointer to it */
PyObject *object_new(int type)
{
    PyObject *obj = PyDict_New();
    if(type==1) {
        PyDict_SetItem(obj, pystr_emails, PyList_New(0));
        PyDict_SetItem(obj, pystr_phones, PyList_New(0));
    }
    return obj;
}

/* set attribute of object or append to list */
void object_add_field(PyObject* obj, int type, int field_id, PyObject* child)
{
    // printf("Setting or appending attribute.. %d.%d\n", type, field_id);
    if(type==1 && field_id==1) {
        PyDict_SetItem(obj, pystr_name, child);
    }

    if(type==1 && field_id==12) {
        PyDict_SetItem(obj, pystr_note, child);
    }

    if(type==1 && field_id==5) {
        PyObject* list;
        list = PyDict_GetItem(obj, pystr_emails);
        PyList_Append(list, child);
    }

    if(type==1 && field_id==6) {
        PyObject* list;
        list = PyDict_GetItem(obj, pystr_phones);
        PyList_Append(list, child);
    }

    if(type==2 && field_id==1) {
        PyDict_SetItem(obj, pystr_email, child);
    }

    if(type==3 && field_id==1) {
        PyDict_SetItem(obj, pystr_phone, child);
    }

    if(type==4 && field_id==1) {
        PyDict_SetItem(obj, pystr_display_name, child);
    }

    if(type==4 && field_id==3) {
        PyDict_SetItem(obj, pystr_first, child);
    }

    if(type==4 && field_id==4) {
        PyDict_SetItem(obj, pystr_last, child);
    }


}

struct context {
    PyObject *node;
    int type;
    int field_id;
    char *end_idx;
} stack[128];
int top = 0;

void* decode(char *msg, int type)
{
//    printf("Len : %d\n", strlen(msg));

    char *c = msg;
    // create root object
    top = 1;
    stack[top].node = object_new(type);
    stack[top].type = type;
    stack[top].end_idx = c + strlen(msg);

    while(*c) {

        unsigned long tag, len;
        int type = -1;

        c = get_tag(c, &tag);
        c = get_msg_len(c, &len);
        type = get_type(stack[top].type, tag);

        // printf("Tag %d Len %d Type = %d\n", tag, len, type);

        if(type==-1) {
            // unknown field, skip it..
            c = c + len;
        } else if(type==0) {

            // primitive.. just set attribute
            char tmp = c[len];
            c[len] = '\0';
            object_add_field(stack[top].node, stack[top].type, tag, PyString_FromString(c));
            c[len] = tmp;

            c = c + len;
        } else {
            // create new node and push it to stack
            // printf("Pushing...\n");
            top++;
            stack[top].node = object_new(type);
            stack[top].type = type;
            stack[top].field_id = tag;
            stack[top].end_idx = c + len;
        }

        // finished? Pop from stack?
        while( top && c >= stack[top].end_idx ) {
            if( top-1 ) {
                object_add_field(stack[top-1].node, stack[top-1].type, stack[top].field_id, stack[top].node);
                // printf("Popping...\n");
                top--;
            } else {
                return stack[top].node;
            }
        }
    }
    return NULL;
}


static PyObject* py_decode(PyObject *self, PyObject *args)
{
    int type;
    char *msg;

    if (PyArg_ParseTuple(args, "is", &type, &msg)) {
         return decode(msg, type);
    }

    return NULL;
}

static PyMethodDef methods[] = {
    {"decode",  py_decode, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};



typedef struct {
  PyObject_HEAD

  int decoded;
  int msg_type;
  char *msg_data;
  PyObject *fields;
} PBMsg;

static PyMethodDef pbmsg_methods[] = {
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static PBMsg* PBMsg_new(int msg_type, char *msg_data);
static PyObject* PBMsg_getattr(PyObject  *o, PyObject  *attr_name);

static int PBMsg_init(PBMsg *self, PyObject *arg, PyObject *kwds) { return 0; }
static void PBMsg_dealloc(PBMsg *self)  { self->ob_type->tp_free((PyObject*)self); }
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
  "ctest.PBMsg",           /*tp_name*/
  sizeof(PBMsg),              /*tp_basicsize*/
  0,                            /*tp_itemsize*/
  /* methods */
  (destructor)PBMsg_dealloc,  /*tp_dealloc*/
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
  PBMsg_getattr,                            /*tp_getattro*/
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
  pbmsg_methods,              /*tp_methods*/
  0,                            /*tp_members*/
  0,                            /*tp_getset*/
  0,                            /*tp_base*/
  0,                            /*tp_dict*/
  0,                            /*tp_descr_get*/
  0,                            /*tp_descr_set*/
  0,                            /*tp_dictoffset*/
  (initproc)PBMsg_init,       /*tp_init*/
  0,                            /*tp_alloc*/
  PBMsg_new_py,                  /*tp_new*/
  0,                            /*tp_free*/
  0,                            /*tp_is_gc*/
};



static PBMsg* PBMsg_new(int msg_type, char *msg_data)
{
        PBMsg *self;
        self = (PBMsg *)PBMsgType.tp_alloc(&PBMsgType, 0);
        if (self == NULL) return NULL;

        self->decoded = 0;
        self->msg_type = msg_type;
        self->msg_data = malloc(strlen(msg_data)+1);
        strcpy(self->msg_data, msg_data);

        return self;
}

void PBMsg_decode(PBMsg *node)
{
    // printf("\nDecoding node... type=%d\n", node->msg_type);

    // Traverse childs...
    char *c = node->msg_data;
    node->fields = object_new(node->msg_type);

    while(*c) {

        unsigned long tag, len;
        int type = -1;

        c = get_tag(c, &tag);
        c = get_msg_len(c, &len);
        type = get_type(node->msg_type, tag);

        // printf("Tag %d Len %d Type = %d\n", tag, len, type);

        if(type==-1) {
            // unknown field, skip it..
        } else if(type==0) {
            // primitive.. just set attribute
            char tmp = c[len];
            c[len] = '\0';
            object_add_field(node->fields, node->msg_type, tag, PyString_FromString(c));
            c[len] = tmp;
        } else {
            // Create unparsed child node..
            char tmp = c[len];
            c[len] = '\0';
            PBMsg *child = PBMsg_new(type, c);
            c[len] = tmp;

            object_add_field(node->fields, node->msg_type, tag, child);
        }

        c += len;
    }
 
    node->decoded = 1;
}


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


    pystr_emails = PyString_FromString("emails");
    pystr_phones = PyString_FromString("phones");
    pystr_name = PyString_FromString("name");
    pystr_note = PyString_FromString("note");
    pystr_first = PyString_FromString("first");
    pystr_last = PyString_FromString("last");
    pystr_display_name = PyString_FromString("display_name");
    pystr_email = PyString_FromString("email");
    pystr_phone = PyString_FromString("phone");


    return;
}


