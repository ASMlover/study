// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: echo.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "echo.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace echo {
class EchoRequestDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<EchoRequest> {
} _EchoRequest_default_instance_;
class EchoResponseDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<EchoResponse> {
} _EchoResponse_default_instance_;

namespace protobuf_echo_2eproto {


namespace {

::google::protobuf::Metadata file_level_metadata[2];
const ::google::protobuf::ServiceDescriptor* file_level_service_descriptors[1];

}  // namespace

const ::google::protobuf::uint32 TableStruct::offsets[] = {
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(EchoRequest, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(EchoRequest, request_),
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(EchoResponse, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(EchoResponse, response_),
};

static const ::google::protobuf::internal::MigrationSchema schemas[] = {
  { 0, -1, sizeof(EchoRequest)},
  { 5, -1, sizeof(EchoResponse)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_EchoRequest_default_instance_),
  reinterpret_cast<const ::google::protobuf::Message*>(&_EchoResponse_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "echo.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, NULL, file_level_service_descriptors);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 2);
}

}  // namespace

void TableStruct::Shutdown() {
  _EchoRequest_default_instance_.Shutdown();
  delete file_level_metadata[0].reflection;
  _EchoResponse_default_instance_.Shutdown();
  delete file_level_metadata[1].reflection;
}

void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  _EchoRequest_default_instance_.DefaultConstruct();
  _EchoResponse_default_instance_.DefaultConstruct();
}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] = {
      "\n\necho.proto\022\004echo\"\036\n\013EchoRequest\022\017\n\007req"
      "uest\030\001 \001(\t\" \n\014EchoResponse\022\020\n\010response\030\001"
      " \001(\t2\?\n\013EchoService\0220\n\007do_echo\022\021.echo.Ec"
      "hoRequest\032\022.echo.EchoResponseB\003\200\001\001b\006prot"
      "o3"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 162);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "echo.proto", &protobuf_RegisterTypes);
  ::google::protobuf::internal::OnShutdown(&TableStruct::Shutdown);
}

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;

}  // namespace protobuf_echo_2eproto


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int EchoRequest::kRequestFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

EchoRequest::EchoRequest()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_echo_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:echo.EchoRequest)
}
EchoRequest::EchoRequest(const EchoRequest& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  request_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.request().size() > 0) {
    request_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.request_);
  }
  // @@protoc_insertion_point(copy_constructor:echo.EchoRequest)
}

void EchoRequest::SharedCtor() {
  request_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  _cached_size_ = 0;
}

EchoRequest::~EchoRequest() {
  // @@protoc_insertion_point(destructor:echo.EchoRequest)
  SharedDtor();
}

void EchoRequest::SharedDtor() {
  request_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void EchoRequest::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* EchoRequest::descriptor() {
  protobuf_echo_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_echo_2eproto::file_level_metadata[0].descriptor;
}

const EchoRequest& EchoRequest::default_instance() {
  protobuf_echo_2eproto::InitDefaults();
  return *internal_default_instance();
}

EchoRequest* EchoRequest::New(::google::protobuf::Arena* arena) const {
  EchoRequest* n = new EchoRequest;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void EchoRequest::Clear() {
// @@protoc_insertion_point(message_clear_start:echo.EchoRequest)
  request_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

bool EchoRequest::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:echo.EchoRequest)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // string request = 1;
      case 1: {
        if (tag == 10u) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_request()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->request().data(), this->request().length(),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "echo.EchoRequest.request"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:echo.EchoRequest)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:echo.EchoRequest)
  return false;
#undef DO_
}

void EchoRequest::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:echo.EchoRequest)
  // string request = 1;
  if (this->request().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->request().data(), this->request().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "echo.EchoRequest.request");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->request(), output);
  }

  // @@protoc_insertion_point(serialize_end:echo.EchoRequest)
}

::google::protobuf::uint8* EchoRequest::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic;  // Unused
  // @@protoc_insertion_point(serialize_to_array_start:echo.EchoRequest)
  // string request = 1;
  if (this->request().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->request().data(), this->request().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "echo.EchoRequest.request");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->request(), target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:echo.EchoRequest)
  return target;
}

size_t EchoRequest::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:echo.EchoRequest)
  size_t total_size = 0;

  // string request = 1;
  if (this->request().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->request());
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void EchoRequest::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:echo.EchoRequest)
  GOOGLE_DCHECK_NE(&from, this);
  const EchoRequest* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const EchoRequest>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:echo.EchoRequest)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:echo.EchoRequest)
    MergeFrom(*source);
  }
}

void EchoRequest::MergeFrom(const EchoRequest& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:echo.EchoRequest)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  if (from.request().size() > 0) {

    request_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.request_);
  }
}

void EchoRequest::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:echo.EchoRequest)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void EchoRequest::CopyFrom(const EchoRequest& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:echo.EchoRequest)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool EchoRequest::IsInitialized() const {
  return true;
}

void EchoRequest::Swap(EchoRequest* other) {
  if (other == this) return;
  InternalSwap(other);
}
void EchoRequest::InternalSwap(EchoRequest* other) {
  request_.Swap(&other->request_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata EchoRequest::GetMetadata() const {
  protobuf_echo_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_echo_2eproto::file_level_metadata[0];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// EchoRequest

// string request = 1;
void EchoRequest::clear_request() {
  request_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
const ::std::string& EchoRequest::request() const {
  // @@protoc_insertion_point(field_get:echo.EchoRequest.request)
  return request_.GetNoArena();
}
void EchoRequest::set_request(const ::std::string& value) {
  
  request_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:echo.EchoRequest.request)
}
#if LANG_CXX11
void EchoRequest::set_request(::std::string&& value) {
  
  request_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:echo.EchoRequest.request)
}
#endif
void EchoRequest::set_request(const char* value) {
  
  request_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:echo.EchoRequest.request)
}
void EchoRequest::set_request(const char* value, size_t size) {
  
  request_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:echo.EchoRequest.request)
}
::std::string* EchoRequest::mutable_request() {
  
  // @@protoc_insertion_point(field_mutable:echo.EchoRequest.request)
  return request_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
::std::string* EchoRequest::release_request() {
  // @@protoc_insertion_point(field_release:echo.EchoRequest.request)
  
  return request_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
void EchoRequest::set_allocated_request(::std::string* request) {
  if (request != NULL) {
    
  } else {
    
  }
  request_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), request);
  // @@protoc_insertion_point(field_set_allocated:echo.EchoRequest.request)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int EchoResponse::kResponseFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

EchoResponse::EchoResponse()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_echo_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:echo.EchoResponse)
}
EchoResponse::EchoResponse(const EchoResponse& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  response_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.response().size() > 0) {
    response_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.response_);
  }
  // @@protoc_insertion_point(copy_constructor:echo.EchoResponse)
}

void EchoResponse::SharedCtor() {
  response_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  _cached_size_ = 0;
}

EchoResponse::~EchoResponse() {
  // @@protoc_insertion_point(destructor:echo.EchoResponse)
  SharedDtor();
}

void EchoResponse::SharedDtor() {
  response_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void EchoResponse::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* EchoResponse::descriptor() {
  protobuf_echo_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_echo_2eproto::file_level_metadata[1].descriptor;
}

const EchoResponse& EchoResponse::default_instance() {
  protobuf_echo_2eproto::InitDefaults();
  return *internal_default_instance();
}

EchoResponse* EchoResponse::New(::google::protobuf::Arena* arena) const {
  EchoResponse* n = new EchoResponse;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void EchoResponse::Clear() {
// @@protoc_insertion_point(message_clear_start:echo.EchoResponse)
  response_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

bool EchoResponse::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:echo.EchoResponse)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // string response = 1;
      case 1: {
        if (tag == 10u) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_response()));
          DO_(::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
            this->response().data(), this->response().length(),
            ::google::protobuf::internal::WireFormatLite::PARSE,
            "echo.EchoResponse.response"));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:echo.EchoResponse)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:echo.EchoResponse)
  return false;
#undef DO_
}

void EchoResponse::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:echo.EchoResponse)
  // string response = 1;
  if (this->response().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->response().data(), this->response().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "echo.EchoResponse.response");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->response(), output);
  }

  // @@protoc_insertion_point(serialize_end:echo.EchoResponse)
}

::google::protobuf::uint8* EchoResponse::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic;  // Unused
  // @@protoc_insertion_point(serialize_to_array_start:echo.EchoResponse)
  // string response = 1;
  if (this->response().size() > 0) {
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
      this->response().data(), this->response().length(),
      ::google::protobuf::internal::WireFormatLite::SERIALIZE,
      "echo.EchoResponse.response");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->response(), target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:echo.EchoResponse)
  return target;
}

size_t EchoResponse::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:echo.EchoResponse)
  size_t total_size = 0;

  // string response = 1;
  if (this->response().size() > 0) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->response());
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void EchoResponse::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:echo.EchoResponse)
  GOOGLE_DCHECK_NE(&from, this);
  const EchoResponse* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const EchoResponse>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:echo.EchoResponse)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:echo.EchoResponse)
    MergeFrom(*source);
  }
}

void EchoResponse::MergeFrom(const EchoResponse& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:echo.EchoResponse)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  if (from.response().size() > 0) {

    response_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.response_);
  }
}

void EchoResponse::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:echo.EchoResponse)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void EchoResponse::CopyFrom(const EchoResponse& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:echo.EchoResponse)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool EchoResponse::IsInitialized() const {
  return true;
}

void EchoResponse::Swap(EchoResponse* other) {
  if (other == this) return;
  InternalSwap(other);
}
void EchoResponse::InternalSwap(EchoResponse* other) {
  response_.Swap(&other->response_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata EchoResponse::GetMetadata() const {
  protobuf_echo_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_echo_2eproto::file_level_metadata[1];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// EchoResponse

// string response = 1;
void EchoResponse::clear_response() {
  response_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
const ::std::string& EchoResponse::response() const {
  // @@protoc_insertion_point(field_get:echo.EchoResponse.response)
  return response_.GetNoArena();
}
void EchoResponse::set_response(const ::std::string& value) {
  
  response_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:echo.EchoResponse.response)
}
#if LANG_CXX11
void EchoResponse::set_response(::std::string&& value) {
  
  response_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:echo.EchoResponse.response)
}
#endif
void EchoResponse::set_response(const char* value) {
  
  response_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:echo.EchoResponse.response)
}
void EchoResponse::set_response(const char* value, size_t size) {
  
  response_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:echo.EchoResponse.response)
}
::std::string* EchoResponse::mutable_response() {
  
  // @@protoc_insertion_point(field_mutable:echo.EchoResponse.response)
  return response_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
::std::string* EchoResponse::release_response() {
  // @@protoc_insertion_point(field_release:echo.EchoResponse.response)
  
  return response_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
void EchoResponse::set_allocated_response(::std::string* response) {
  if (response != NULL) {
    
  } else {
    
  }
  response_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), response);
  // @@protoc_insertion_point(field_set_allocated:echo.EchoResponse.response)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// ===================================================================

EchoService::~EchoService() {}

const ::google::protobuf::ServiceDescriptor* EchoService::descriptor() {
  protobuf_echo_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_echo_2eproto::file_level_service_descriptors[0];
}

const ::google::protobuf::ServiceDescriptor* EchoService::GetDescriptor() {
  return descriptor();
}

void EchoService::do_echo(::google::protobuf::RpcController* controller,
                         const ::echo::EchoRequest*,
                         ::echo::EchoResponse*,
                         ::google::protobuf::Closure* done) {
  controller->SetFailed("Method do_echo() not implemented.");
  done->Run();
}

void EchoService::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                             ::google::protobuf::RpcController* controller,
                             const ::google::protobuf::Message* request,
                             ::google::protobuf::Message* response,
                             ::google::protobuf::Closure* done) {
  GOOGLE_DCHECK_EQ(method->service(), protobuf_echo_2eproto::file_level_service_descriptors[0]);
  switch(method->index()) {
    case 0:
      do_echo(controller,
             ::google::protobuf::down_cast<const ::echo::EchoRequest*>(request),
             ::google::protobuf::down_cast< ::echo::EchoResponse*>(response),
             done);
      break;
    default:
      GOOGLE_LOG(FATAL) << "Bad method index; this should never happen.";
      break;
  }
}

const ::google::protobuf::Message& EchoService::GetRequestPrototype(
    const ::google::protobuf::MethodDescriptor* method) const {
  GOOGLE_DCHECK_EQ(method->service(), descriptor());
  switch(method->index()) {
    case 0:
      return ::echo::EchoRequest::default_instance();
    default:
      GOOGLE_LOG(FATAL) << "Bad method index; this should never happen.";
      return *::google::protobuf::MessageFactory::generated_factory()
          ->GetPrototype(method->input_type());
  }
}

const ::google::protobuf::Message& EchoService::GetResponsePrototype(
    const ::google::protobuf::MethodDescriptor* method) const {
  GOOGLE_DCHECK_EQ(method->service(), descriptor());
  switch(method->index()) {
    case 0:
      return ::echo::EchoResponse::default_instance();
    default:
      GOOGLE_LOG(FATAL) << "Bad method index; this should never happen.";
      return *::google::protobuf::MessageFactory::generated_factory()
          ->GetPrototype(method->output_type());
  }
}

EchoService_Stub::EchoService_Stub(::google::protobuf::RpcChannel* channel)
  : channel_(channel), owns_channel_(false) {}
EchoService_Stub::EchoService_Stub(
    ::google::protobuf::RpcChannel* channel,
    ::google::protobuf::Service::ChannelOwnership ownership)
  : channel_(channel),
    owns_channel_(ownership == ::google::protobuf::Service::STUB_OWNS_CHANNEL) {}
EchoService_Stub::~EchoService_Stub() {
  if (owns_channel_) delete channel_;
}

void EchoService_Stub::do_echo(::google::protobuf::RpcController* controller,
                              const ::echo::EchoRequest* request,
                              ::echo::EchoResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(0),
                       controller, request, response, done);
}

// @@protoc_insertion_point(namespace_scope)

}  // namespace echo

// @@protoc_insertion_point(global_scope)
