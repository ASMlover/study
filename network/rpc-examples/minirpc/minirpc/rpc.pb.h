// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: rpc.proto

#ifndef PROTOBUF_rpc_2eproto__INCLUDED
#define PROTOBUF_rpc_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3002000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3002000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
namespace minirpc {
class RpcMessage;
class RpcMessageDefaultTypeInternal;
extern RpcMessageDefaultTypeInternal _RpcMessage_default_instance_;
}  // namespace minirpc

namespace minirpc {

namespace protobuf_rpc_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::uint32 offsets[];
  static void InitDefaultsImpl();
  static void Shutdown();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_rpc_2eproto

enum MessageType {
  MT_ERROR = 0,
  MT_REQUEST = 1,
  MT_RESPONSE = 2,
  MessageType_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  MessageType_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool MessageType_IsValid(int value);
const MessageType MessageType_MIN = MT_ERROR;
const MessageType MessageType_MAX = MT_RESPONSE;
const int MessageType_ARRAYSIZE = MessageType_MAX + 1;

const ::google::protobuf::EnumDescriptor* MessageType_descriptor();
inline const ::std::string& MessageType_Name(MessageType value) {
  return ::google::protobuf::internal::NameOfEnum(
    MessageType_descriptor(), value);
}
inline bool MessageType_Parse(
    const ::std::string& name, MessageType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<MessageType>(
    MessageType_descriptor(), name, value);
}
enum ErrorCode {
  EC_WRONG_PROTO = 0,
  EC_NO_SERVICE = 1,
  EC_NO_METHOD = 2,
  EC_INVALID_REQUEST = 3,
  EC_INVALID_RESPONSE = 4,
  ErrorCode_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  ErrorCode_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool ErrorCode_IsValid(int value);
const ErrorCode ErrorCode_MIN = EC_WRONG_PROTO;
const ErrorCode ErrorCode_MAX = EC_INVALID_RESPONSE;
const int ErrorCode_ARRAYSIZE = ErrorCode_MAX + 1;

const ::google::protobuf::EnumDescriptor* ErrorCode_descriptor();
inline const ::std::string& ErrorCode_Name(ErrorCode value) {
  return ::google::protobuf::internal::NameOfEnum(
    ErrorCode_descriptor(), value);
}
inline bool ErrorCode_Parse(
    const ::std::string& name, ErrorCode* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ErrorCode>(
    ErrorCode_descriptor(), name, value);
}
// ===================================================================

class RpcMessage : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:minirpc.RpcMessage) */ {
 public:
  RpcMessage();
  virtual ~RpcMessage();

  RpcMessage(const RpcMessage& from);

  inline RpcMessage& operator=(const RpcMessage& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RpcMessage& default_instance();

  static inline const RpcMessage* internal_default_instance() {
    return reinterpret_cast<const RpcMessage*>(
               &_RpcMessage_default_instance_);
  }

  void Swap(RpcMessage* other);

  // implements Message ----------------------------------------------

  inline RpcMessage* New() const PROTOBUF_FINAL { return New(NULL); }

  RpcMessage* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const RpcMessage& from);
  void MergeFrom(const RpcMessage& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output)
      const PROTOBUF_FINAL {
    return InternalSerializeWithCachedSizesToArray(
        ::google::protobuf::io::CodedOutputStream::IsDefaultSerializationDeterministic(), output);
  }
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(RpcMessage* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string service = 3;
  void clear_service();
  static const int kServiceFieldNumber = 3;
  const ::std::string& service() const;
  void set_service(const ::std::string& value);
  #if LANG_CXX11
  void set_service(::std::string&& value);
  #endif
  void set_service(const char* value);
  void set_service(const char* value, size_t size);
  ::std::string* mutable_service();
  ::std::string* release_service();
  void set_allocated_service(::std::string* service);

  // string method = 4;
  void clear_method();
  static const int kMethodFieldNumber = 4;
  const ::std::string& method() const;
  void set_method(const ::std::string& value);
  #if LANG_CXX11
  void set_method(::std::string&& value);
  #endif
  void set_method(const char* value);
  void set_method(const char* value, size_t size);
  ::std::string* mutable_method();
  ::std::string* release_method();
  void set_allocated_method(::std::string* method);

  // bytes request = 5;
  void clear_request();
  static const int kRequestFieldNumber = 5;
  const ::std::string& request() const;
  void set_request(const ::std::string& value);
  #if LANG_CXX11
  void set_request(::std::string&& value);
  #endif
  void set_request(const char* value);
  void set_request(const void* value, size_t size);
  ::std::string* mutable_request();
  ::std::string* release_request();
  void set_allocated_request(::std::string* request);

  // bytes response = 6;
  void clear_response();
  static const int kResponseFieldNumber = 6;
  const ::std::string& response() const;
  void set_response(const ::std::string& value);
  #if LANG_CXX11
  void set_response(::std::string&& value);
  #endif
  void set_response(const char* value);
  void set_response(const void* value, size_t size);
  ::std::string* mutable_response();
  ::std::string* release_response();
  void set_allocated_response(::std::string* response);

  // fixed64 id = 2;
  void clear_id();
  static const int kIdFieldNumber = 2;
  ::google::protobuf::uint64 id() const;
  void set_id(::google::protobuf::uint64 value);

  // .minirpc.MessageType type = 1;
  void clear_type();
  static const int kTypeFieldNumber = 1;
  ::minirpc::MessageType type() const;
  void set_type(::minirpc::MessageType value);

  // .minirpc.ErrorCode ec = 7;
  void clear_ec();
  static const int kEcFieldNumber = 7;
  ::minirpc::ErrorCode ec() const;
  void set_ec(::minirpc::ErrorCode value);

  // @@protoc_insertion_point(class_scope:minirpc.RpcMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr service_;
  ::google::protobuf::internal::ArenaStringPtr method_;
  ::google::protobuf::internal::ArenaStringPtr request_;
  ::google::protobuf::internal::ArenaStringPtr response_;
  ::google::protobuf::uint64 id_;
  int type_;
  int ec_;
  mutable int _cached_size_;
  friend struct  protobuf_rpc_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// RpcMessage

// .minirpc.MessageType type = 1;
inline void RpcMessage::clear_type() {
  type_ = 0;
}
inline ::minirpc::MessageType RpcMessage::type() const {
  // @@protoc_insertion_point(field_get:minirpc.RpcMessage.type)
  return static_cast< ::minirpc::MessageType >(type_);
}
inline void RpcMessage::set_type(::minirpc::MessageType value) {
  
  type_ = value;
  // @@protoc_insertion_point(field_set:minirpc.RpcMessage.type)
}

// fixed64 id = 2;
inline void RpcMessage::clear_id() {
  id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 RpcMessage::id() const {
  // @@protoc_insertion_point(field_get:minirpc.RpcMessage.id)
  return id_;
}
inline void RpcMessage::set_id(::google::protobuf::uint64 value) {
  
  id_ = value;
  // @@protoc_insertion_point(field_set:minirpc.RpcMessage.id)
}

// string service = 3;
inline void RpcMessage::clear_service() {
  service_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcMessage::service() const {
  // @@protoc_insertion_point(field_get:minirpc.RpcMessage.service)
  return service_.GetNoArena();
}
inline void RpcMessage::set_service(const ::std::string& value) {
  
  service_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:minirpc.RpcMessage.service)
}
#if LANG_CXX11
inline void RpcMessage::set_service(::std::string&& value) {
  
  service_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:minirpc.RpcMessage.service)
}
#endif
inline void RpcMessage::set_service(const char* value) {
  
  service_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:minirpc.RpcMessage.service)
}
inline void RpcMessage::set_service(const char* value, size_t size) {
  
  service_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:minirpc.RpcMessage.service)
}
inline ::std::string* RpcMessage::mutable_service() {
  
  // @@protoc_insertion_point(field_mutable:minirpc.RpcMessage.service)
  return service_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcMessage::release_service() {
  // @@protoc_insertion_point(field_release:minirpc.RpcMessage.service)
  
  return service_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcMessage::set_allocated_service(::std::string* service) {
  if (service != NULL) {
    
  } else {
    
  }
  service_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), service);
  // @@protoc_insertion_point(field_set_allocated:minirpc.RpcMessage.service)
}

// string method = 4;
inline void RpcMessage::clear_method() {
  method_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcMessage::method() const {
  // @@protoc_insertion_point(field_get:minirpc.RpcMessage.method)
  return method_.GetNoArena();
}
inline void RpcMessage::set_method(const ::std::string& value) {
  
  method_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:minirpc.RpcMessage.method)
}
#if LANG_CXX11
inline void RpcMessage::set_method(::std::string&& value) {
  
  method_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:minirpc.RpcMessage.method)
}
#endif
inline void RpcMessage::set_method(const char* value) {
  
  method_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:minirpc.RpcMessage.method)
}
inline void RpcMessage::set_method(const char* value, size_t size) {
  
  method_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:minirpc.RpcMessage.method)
}
inline ::std::string* RpcMessage::mutable_method() {
  
  // @@protoc_insertion_point(field_mutable:minirpc.RpcMessage.method)
  return method_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcMessage::release_method() {
  // @@protoc_insertion_point(field_release:minirpc.RpcMessage.method)
  
  return method_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcMessage::set_allocated_method(::std::string* method) {
  if (method != NULL) {
    
  } else {
    
  }
  method_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), method);
  // @@protoc_insertion_point(field_set_allocated:minirpc.RpcMessage.method)
}

// bytes request = 5;
inline void RpcMessage::clear_request() {
  request_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcMessage::request() const {
  // @@protoc_insertion_point(field_get:minirpc.RpcMessage.request)
  return request_.GetNoArena();
}
inline void RpcMessage::set_request(const ::std::string& value) {
  
  request_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:minirpc.RpcMessage.request)
}
#if LANG_CXX11
inline void RpcMessage::set_request(::std::string&& value) {
  
  request_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:minirpc.RpcMessage.request)
}
#endif
inline void RpcMessage::set_request(const char* value) {
  
  request_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:minirpc.RpcMessage.request)
}
inline void RpcMessage::set_request(const void* value, size_t size) {
  
  request_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:minirpc.RpcMessage.request)
}
inline ::std::string* RpcMessage::mutable_request() {
  
  // @@protoc_insertion_point(field_mutable:minirpc.RpcMessage.request)
  return request_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcMessage::release_request() {
  // @@protoc_insertion_point(field_release:minirpc.RpcMessage.request)
  
  return request_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcMessage::set_allocated_request(::std::string* request) {
  if (request != NULL) {
    
  } else {
    
  }
  request_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), request);
  // @@protoc_insertion_point(field_set_allocated:minirpc.RpcMessage.request)
}

// bytes response = 6;
inline void RpcMessage::clear_response() {
  response_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcMessage::response() const {
  // @@protoc_insertion_point(field_get:minirpc.RpcMessage.response)
  return response_.GetNoArena();
}
inline void RpcMessage::set_response(const ::std::string& value) {
  
  response_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:minirpc.RpcMessage.response)
}
#if LANG_CXX11
inline void RpcMessage::set_response(::std::string&& value) {
  
  response_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:minirpc.RpcMessage.response)
}
#endif
inline void RpcMessage::set_response(const char* value) {
  
  response_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:minirpc.RpcMessage.response)
}
inline void RpcMessage::set_response(const void* value, size_t size) {
  
  response_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:minirpc.RpcMessage.response)
}
inline ::std::string* RpcMessage::mutable_response() {
  
  // @@protoc_insertion_point(field_mutable:minirpc.RpcMessage.response)
  return response_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcMessage::release_response() {
  // @@protoc_insertion_point(field_release:minirpc.RpcMessage.response)
  
  return response_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcMessage::set_allocated_response(::std::string* response) {
  if (response != NULL) {
    
  } else {
    
  }
  response_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), response);
  // @@protoc_insertion_point(field_set_allocated:minirpc.RpcMessage.response)
}

// .minirpc.ErrorCode ec = 7;
inline void RpcMessage::clear_ec() {
  ec_ = 0;
}
inline ::minirpc::ErrorCode RpcMessage::ec() const {
  // @@protoc_insertion_point(field_get:minirpc.RpcMessage.ec)
  return static_cast< ::minirpc::ErrorCode >(ec_);
}
inline void RpcMessage::set_ec(::minirpc::ErrorCode value) {
  
  ec_ = value;
  // @@protoc_insertion_point(field_set:minirpc.RpcMessage.ec)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


}  // namespace minirpc

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::minirpc::MessageType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::minirpc::MessageType>() {
  return ::minirpc::MessageType_descriptor();
}
template <> struct is_proto_enum< ::minirpc::ErrorCode> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::minirpc::ErrorCode>() {
  return ::minirpc::ErrorCode_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_rpc_2eproto__INCLUDED
