from google.protobuf import descriptor_pb2

fds = descriptor_pb2.FileDescriptorSet()
fds.ParseFromString(open("connexio.pb2").read())

message_types = dict()
message_type_ids = dict()

i = 32


print "Message types"
for mt in fds.file[0].message_type:
    message_types[mt.name] = mt
    message_type_ids[mt.name] = i
    print i, mt.name
    i += 1

def get_type_number(f):
    global message_types

    if f.type == descriptor_pb2.FieldDescriptorProto.TYPE_STRING:
        return 0;

    if f.type == descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE:
        return message_type_ids[f.type_name.split(".")[-1]]


print "Table: "
for (mt_name, mt) in message_types.items():
    mt_id = message_type_ids[mt_name]
    i = 0
    for f in mt.field:
        print "flist[%d][%d] = %d;" % (mt_id, i, f.number)
        print "ft[%d][%d].name_str = PyString_FromString(\"%s\");" % (mt_id, f.number, f.name)
        print "ft[%d][%d].repeated = %d;" % (mt_id, f.number, 1 if f.label == 3 else 0 )
        print "ft[%d][%d].type = %d;" % (mt_id, f.number, get_type_number(f) )
        i += 1
    print "flist[%d][%d] = -1;\n" % (mt_id, i)

print message_types

