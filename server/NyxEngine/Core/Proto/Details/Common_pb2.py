# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: Common.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='Common.proto',
  package='Nyx.Core',
  syntax='proto3',
  serialized_pb=_b('\n\x0c\x43ommon.proto\x12\x08Nyx.Core\"\x06\n\x04Void\"\xd1\x01\n\x0e\x43onnectRequest\x12\x0e\n\x06routes\x18\x01 \x01(\x0c\x12\x32\n\x04type\x18\x02 \x01(\x0e\x32$.Nyx.Core.ConnectRequest.RequestType\x12\x11\n\tdevice_id\x18\x03 \x01(\x0c\x12\x11\n\tentity_id\x18\x04 \x01(\x0c\x12\x10\n\x08\x61uth_msg\x18\x05 \x01(\x0c\"C\n\x0bRequestType\x12\x11\n\rNEWCONNECTION\x10\x00\x12\x10\n\x0cRECONNECTION\x10\x01\x12\x0f\n\x0b\x42IND_AVATAR\x10\x02\"\xf5\x01\n\x0f\x43onnectResponse\x12\x0e\n\x06routes\x18\x01 \x01(\x0c\x12\x34\n\x04type\x18\x02 \x01(\x0e\x32&.Nyx.Core.ConnectResponse.ResponseType\x12\x11\n\tentity_id\x18\x03 \x01(\x0c\x12\x11\n\textra_msg\x18\x04 \x01(\x0c\"v\n\x0cResponseType\x12\x08\n\x04\x42USY\x10\x00\x12\r\n\tCONNECTED\x10\x01\x12\x13\n\x0fRECONNECTION_OK\x10\x02\x12\x15\n\x11RECONNECTION_FAIL\x10\x03\x12\r\n\tFORBIDDEN\x10\x04\x12\x12\n\x0eMAX_CONNECTION\x10\x05\"&\n\x08Md5Index\x12\x0b\n\x03md5\x18\x01 \x01(\x0c\x12\r\n\x05index\x18\x02 \x01(\x11\"|\n\rEntityMessage\x12\x0e\n\x06routes\x18\x01 \x01(\x0c\x12\x11\n\tentity_id\x18\x02 \x01(\x0c\x12\"\n\x06method\x18\x03 \x01(\x0b\x32\x12.Nyx.Core.Md5Index\x12\x12\n\nparameters\x18\x04 \x01(\x0c\x12\x10\n\x08reliable\x18\x05 \x01(\x08\"o\n\x13GlobalEntityMessage\x12\x0e\n\x06target\x18\x01 \x01(\x0c\x12\"\n\x06method\x18\x02 \x01(\x0b\x32\x12.Nyx.Core.Md5Index\x12\x12\n\nparameters\x18\x03 \x01(\x0c\x12\x10\n\x08reliable\x18\x04 \x01(\x08\"`\n\nEntityInfo\x12\x0e\n\x06routes\x18\x01 \x01(\x0c\x12 \n\x04type\x18\x02 \x01(\x0b\x32\x12.Nyx.Core.Md5Index\x12\x11\n\tentity_id\x18\x03 \x01(\x0c\x12\r\n\x05infos\x18\x04 \x01(\x0c\",\n\x0bOutbandInfo\x12\x0e\n\x06routes\x18\x01 \x01(\x0c\x12\r\n\x05infos\x18\x02 \x01(\x0c\"\xa4\x01\n\nServerInfo\x12\n\n\x02ip\x18\x01 \x01(\x0c\x12\x0c\n\x04port\x18\x02 \x01(\x05\x12\x0b\n\x03sid\x18\x03 \x01(\x05\x12\x12\n\nban_client\x18\x04 \x01(\x08\x12\x34\n\x0bserver_type\x18\x05 \x01(\x0e\x32\x1f.Nyx.Core.ServerInfo.ServerType\"%\n\nServerType\x12\x0b\n\x07INVALID\x10\x00\x12\n\n\x06NORMAL\x10\x01\"M\n\rEntityMailbox\x12\x11\n\tentity_id\x18\x01 \x01(\x0c\x12)\n\x0bserver_info\x18\x02 \x01(\x0b\x32\x14.Nyx.Core.ServerInfoB\x06\x80\x01\x01\x90\x01\x01\x62\x06proto3')
)
_sym_db.RegisterFileDescriptor(DESCRIPTOR)



_CONNECTREQUEST_REQUESTTYPE = _descriptor.EnumDescriptor(
  name='RequestType',
  full_name='Nyx.Core.ConnectRequest.RequestType',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='NEWCONNECTION', index=0, number=0,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='RECONNECTION', index=1, number=1,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='BIND_AVATAR', index=2, number=2,
      options=None,
      type=None),
  ],
  containing_type=None,
  options=None,
  serialized_start=177,
  serialized_end=244,
)
_sym_db.RegisterEnumDescriptor(_CONNECTREQUEST_REQUESTTYPE)

_CONNECTRESPONSE_RESPONSETYPE = _descriptor.EnumDescriptor(
  name='ResponseType',
  full_name='Nyx.Core.ConnectResponse.ResponseType',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='BUSY', index=0, number=0,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='CONNECTED', index=1, number=1,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='RECONNECTION_OK', index=2, number=2,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='RECONNECTION_FAIL', index=3, number=3,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='FORBIDDEN', index=4, number=4,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='MAX_CONNECTION', index=5, number=5,
      options=None,
      type=None),
  ],
  containing_type=None,
  options=None,
  serialized_start=374,
  serialized_end=492,
)
_sym_db.RegisterEnumDescriptor(_CONNECTRESPONSE_RESPONSETYPE)

_SERVERINFO_SERVERTYPE = _descriptor.EnumDescriptor(
  name='ServerType',
  full_name='Nyx.Core.ServerInfo.ServerType',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='INVALID', index=0, number=0,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='NORMAL', index=1, number=1,
      options=None,
      type=None),
  ],
  containing_type=None,
  options=None,
  serialized_start=1045,
  serialized_end=1082,
)
_sym_db.RegisterEnumDescriptor(_SERVERINFO_SERVERTYPE)


_VOID = _descriptor.Descriptor(
  name='Void',
  full_name='Nyx.Core.Void',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=26,
  serialized_end=32,
)


_CONNECTREQUEST = _descriptor.Descriptor(
  name='ConnectRequest',
  full_name='Nyx.Core.ConnectRequest',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='routes', full_name='Nyx.Core.ConnectRequest.routes', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='type', full_name='Nyx.Core.ConnectRequest.type', index=1,
      number=2, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='device_id', full_name='Nyx.Core.ConnectRequest.device_id', index=2,
      number=3, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='entity_id', full_name='Nyx.Core.ConnectRequest.entity_id', index=3,
      number=4, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='auth_msg', full_name='Nyx.Core.ConnectRequest.auth_msg', index=4,
      number=5, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
    _CONNECTREQUEST_REQUESTTYPE,
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=35,
  serialized_end=244,
)


_CONNECTRESPONSE = _descriptor.Descriptor(
  name='ConnectResponse',
  full_name='Nyx.Core.ConnectResponse',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='routes', full_name='Nyx.Core.ConnectResponse.routes', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='type', full_name='Nyx.Core.ConnectResponse.type', index=1,
      number=2, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='entity_id', full_name='Nyx.Core.ConnectResponse.entity_id', index=2,
      number=3, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='extra_msg', full_name='Nyx.Core.ConnectResponse.extra_msg', index=3,
      number=4, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
    _CONNECTRESPONSE_RESPONSETYPE,
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=247,
  serialized_end=492,
)


_MD5INDEX = _descriptor.Descriptor(
  name='Md5Index',
  full_name='Nyx.Core.Md5Index',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='md5', full_name='Nyx.Core.Md5Index.md5', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='index', full_name='Nyx.Core.Md5Index.index', index=1,
      number=2, type=17, cpp_type=1, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=494,
  serialized_end=532,
)


_ENTITYMESSAGE = _descriptor.Descriptor(
  name='EntityMessage',
  full_name='Nyx.Core.EntityMessage',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='routes', full_name='Nyx.Core.EntityMessage.routes', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='entity_id', full_name='Nyx.Core.EntityMessage.entity_id', index=1,
      number=2, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='method', full_name='Nyx.Core.EntityMessage.method', index=2,
      number=3, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='parameters', full_name='Nyx.Core.EntityMessage.parameters', index=3,
      number=4, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='reliable', full_name='Nyx.Core.EntityMessage.reliable', index=4,
      number=5, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=534,
  serialized_end=658,
)


_GLOBALENTITYMESSAGE = _descriptor.Descriptor(
  name='GlobalEntityMessage',
  full_name='Nyx.Core.GlobalEntityMessage',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='target', full_name='Nyx.Core.GlobalEntityMessage.target', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='method', full_name='Nyx.Core.GlobalEntityMessage.method', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='parameters', full_name='Nyx.Core.GlobalEntityMessage.parameters', index=2,
      number=3, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='reliable', full_name='Nyx.Core.GlobalEntityMessage.reliable', index=3,
      number=4, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=660,
  serialized_end=771,
)


_ENTITYINFO = _descriptor.Descriptor(
  name='EntityInfo',
  full_name='Nyx.Core.EntityInfo',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='routes', full_name='Nyx.Core.EntityInfo.routes', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='type', full_name='Nyx.Core.EntityInfo.type', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='entity_id', full_name='Nyx.Core.EntityInfo.entity_id', index=2,
      number=3, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='infos', full_name='Nyx.Core.EntityInfo.infos', index=3,
      number=4, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=773,
  serialized_end=869,
)


_OUTBANDINFO = _descriptor.Descriptor(
  name='OutbandInfo',
  full_name='Nyx.Core.OutbandInfo',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='routes', full_name='Nyx.Core.OutbandInfo.routes', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='infos', full_name='Nyx.Core.OutbandInfo.infos', index=1,
      number=2, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=871,
  serialized_end=915,
)


_SERVERINFO = _descriptor.Descriptor(
  name='ServerInfo',
  full_name='Nyx.Core.ServerInfo',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='ip', full_name='Nyx.Core.ServerInfo.ip', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='port', full_name='Nyx.Core.ServerInfo.port', index=1,
      number=2, type=5, cpp_type=1, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='sid', full_name='Nyx.Core.ServerInfo.sid', index=2,
      number=3, type=5, cpp_type=1, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='ban_client', full_name='Nyx.Core.ServerInfo.ban_client', index=3,
      number=4, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='server_type', full_name='Nyx.Core.ServerInfo.server_type', index=4,
      number=5, type=14, cpp_type=8, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
    _SERVERINFO_SERVERTYPE,
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=918,
  serialized_end=1082,
)


_ENTITYMAILBOX = _descriptor.Descriptor(
  name='EntityMailbox',
  full_name='Nyx.Core.EntityMailbox',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='entity_id', full_name='Nyx.Core.EntityMailbox.entity_id', index=0,
      number=1, type=12, cpp_type=9, label=1,
      has_default_value=False, default_value=_b(""),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='server_info', full_name='Nyx.Core.EntityMailbox.server_info', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=1084,
  serialized_end=1161,
)

_CONNECTREQUEST.fields_by_name['type'].enum_type = _CONNECTREQUEST_REQUESTTYPE
_CONNECTREQUEST_REQUESTTYPE.containing_type = _CONNECTREQUEST
_CONNECTRESPONSE.fields_by_name['type'].enum_type = _CONNECTRESPONSE_RESPONSETYPE
_CONNECTRESPONSE_RESPONSETYPE.containing_type = _CONNECTRESPONSE
_ENTITYMESSAGE.fields_by_name['method'].message_type = _MD5INDEX
_GLOBALENTITYMESSAGE.fields_by_name['method'].message_type = _MD5INDEX
_ENTITYINFO.fields_by_name['type'].message_type = _MD5INDEX
_SERVERINFO.fields_by_name['server_type'].enum_type = _SERVERINFO_SERVERTYPE
_SERVERINFO_SERVERTYPE.containing_type = _SERVERINFO
_ENTITYMAILBOX.fields_by_name['server_info'].message_type = _SERVERINFO
DESCRIPTOR.message_types_by_name['Void'] = _VOID
DESCRIPTOR.message_types_by_name['ConnectRequest'] = _CONNECTREQUEST
DESCRIPTOR.message_types_by_name['ConnectResponse'] = _CONNECTRESPONSE
DESCRIPTOR.message_types_by_name['Md5Index'] = _MD5INDEX
DESCRIPTOR.message_types_by_name['EntityMessage'] = _ENTITYMESSAGE
DESCRIPTOR.message_types_by_name['GlobalEntityMessage'] = _GLOBALENTITYMESSAGE
DESCRIPTOR.message_types_by_name['EntityInfo'] = _ENTITYINFO
DESCRIPTOR.message_types_by_name['OutbandInfo'] = _OUTBANDINFO
DESCRIPTOR.message_types_by_name['ServerInfo'] = _SERVERINFO
DESCRIPTOR.message_types_by_name['EntityMailbox'] = _ENTITYMAILBOX

Void = _reflection.GeneratedProtocolMessageType('Void', (_message.Message,), dict(
  DESCRIPTOR = _VOID,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.Void)
  ))
_sym_db.RegisterMessage(Void)

ConnectRequest = _reflection.GeneratedProtocolMessageType('ConnectRequest', (_message.Message,), dict(
  DESCRIPTOR = _CONNECTREQUEST,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.ConnectRequest)
  ))
_sym_db.RegisterMessage(ConnectRequest)

ConnectResponse = _reflection.GeneratedProtocolMessageType('ConnectResponse', (_message.Message,), dict(
  DESCRIPTOR = _CONNECTRESPONSE,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.ConnectResponse)
  ))
_sym_db.RegisterMessage(ConnectResponse)

Md5Index = _reflection.GeneratedProtocolMessageType('Md5Index', (_message.Message,), dict(
  DESCRIPTOR = _MD5INDEX,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.Md5Index)
  ))
_sym_db.RegisterMessage(Md5Index)

EntityMessage = _reflection.GeneratedProtocolMessageType('EntityMessage', (_message.Message,), dict(
  DESCRIPTOR = _ENTITYMESSAGE,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.EntityMessage)
  ))
_sym_db.RegisterMessage(EntityMessage)

GlobalEntityMessage = _reflection.GeneratedProtocolMessageType('GlobalEntityMessage', (_message.Message,), dict(
  DESCRIPTOR = _GLOBALENTITYMESSAGE,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.GlobalEntityMessage)
  ))
_sym_db.RegisterMessage(GlobalEntityMessage)

EntityInfo = _reflection.GeneratedProtocolMessageType('EntityInfo', (_message.Message,), dict(
  DESCRIPTOR = _ENTITYINFO,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.EntityInfo)
  ))
_sym_db.RegisterMessage(EntityInfo)

OutbandInfo = _reflection.GeneratedProtocolMessageType('OutbandInfo', (_message.Message,), dict(
  DESCRIPTOR = _OUTBANDINFO,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.OutbandInfo)
  ))
_sym_db.RegisterMessage(OutbandInfo)

ServerInfo = _reflection.GeneratedProtocolMessageType('ServerInfo', (_message.Message,), dict(
  DESCRIPTOR = _SERVERINFO,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.ServerInfo)
  ))
_sym_db.RegisterMessage(ServerInfo)

EntityMailbox = _reflection.GeneratedProtocolMessageType('EntityMailbox', (_message.Message,), dict(
  DESCRIPTOR = _ENTITYMAILBOX,
  __module__ = 'Common_pb2'
  # @@protoc_insertion_point(class_scope:Nyx.Core.EntityMailbox)
  ))
_sym_db.RegisterMessage(EntityMailbox)


DESCRIPTOR.has_options = True
DESCRIPTOR._options = _descriptor._ParseOptions(descriptor_pb2.FileOptions(), _b('\200\001\001\220\001\001'))
# @@protoc_insertion_point(module_scope)
