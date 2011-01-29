
from google.protobuf import descriptor_pb2
import sys

fds = descriptor_pb2.FileDescriptorSet()
fds.ParseFromString(open(sys.argv[1]).read())

message_types = dict()
message_type_ids = dict()

i = 32

# Contants - enum values, messagy type id, etc.
consts = "# Automatically generated code. \n\n"
for mt in fds.file[0].message_type:
    message_types[mt.name] = mt
    message_type_ids[mt.name] = i
    consts += "MSG_"+mt.name + " = " + str(i) + "\n"
    i += 1

enum_vals = dict()
for e in fds.file[0].enum_type:
    for v in e.value:
        consts += v.name + " = " + str(v.number) + "\n"

f = open("pb_types.py", "w")
f.write(consts)
f.close()


def get_type_number(f):
    global message_types

    if f.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE:
        return message_type_ids[f.type_name.split(".")[-1]]

    return f.type

table  = "/* automatically generated code. DO NOT EDIT. */\n\n"
table += "void setup_table() {"

for (mt_name, mt) in message_types.items():
    mt_id = message_type_ids[mt_name]
    i = 0
    for f in mt.field:
        table += "\n\tflist[%d][%d] = %d;" % (mt_id, i, f.number)
        table += "\n\tft[%d][%d].name_str = PyString_FromString(\"%s\");" % (mt_id, f.number, f.name)
        table += "\n\tft[%d][%d].repeated = %d;" % (mt_id, f.number, 1 if f.label == 3 else 0 )
        table += "\n\tft[%d][%d].type = %d;" % (mt_id, f.number, get_type_number(f) )
        i += 1
    table += "\n\tflist[%d][%d] = -1;\n" % (mt_id, i)

table += "\n}\n"

f = open("descriptor_compiled.h", "w")
f.write(table)
f.close()

