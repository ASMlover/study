// Copyright (c) 2018 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <sstream>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "../compressor/nyx_compressor.h"
#include "../crypter/nyx_crypter.h"
#include "../nyx_service.h"
#include "nyx_rpc_converter.h"
#include "nyx_rpc_channel.h"

namespace nyx { namespace rpc {

rpc_channel::rpc_channel(base_service* service)
  : wbits_(12)
  , memlevel_(5)
  , service_(service) {
}

rpc_channel::~rpc_channel(void) {
}

bool rpc_channel::on_request(unsigned char channel) {
  auto& request = requests_[channel];
  if (request.get_size() < sizeof(method_index_type)) {
    service_->disconnect();
    return false;
  }

  method_index_type index;
  request.data_rbuffer().read(reinterpret_cast<char*>(&index), sizeof(index));
  if (index >= service_->get_descriptor()->method_count()) {
    service_->disconnect();
    return false;
  }

  auto* method = service_->get_descriptor()->method(index);
  auto* msg = service_->get_request_prototype(method).New();
  if (!msg->ParseFromIstream(&request.data_rbuffer())) {
    service_->disconnect();
    return false;
  }

  service_->call_method(method, nullptr, msg, nullptr, nullptr);
  return true;
}

void rpc_channel::CallMethod(
    const pb::MethodDescriptor* method,
    pb::RpcController* controller,
    const pb::Message* request,
    pb::Message* response,
    pb::Closure* done) {
  std::ostringstream oss;
  unsigned int total_len = 0;
  oss.write(reinterpret_cast<const char*>(&total_len), sizeof(total_len));
  method_index_type index = method->index();
  oss.write(reinterpret_cast<const char*>(&index), sizeof(index));
  if (!request->SerializeToOstream(&oss)) {
    service_->disconnect();
    return;
  }

  total_len = oss.tellp();
  auto data_len = total_len - sizeof(total_len);
  oss.seekp(0, std::ios_base::beg);
  oss.write(reinterpret_cast<const char*>(&data_len), sizeof(data_len));
  oss.seekp(total_len);

  auto str_data = std::make_shared<std::string>(oss.str());
  if (traverse_) {
    traverse_->set_msg(str_data);
    traverse_.reset();
  }

  bool reliable{true};
  unsigned char channel = kDefaultChannelId;
  if (controller) {
    auto* ctrl = pb::down_cast<rpc_controller*>(controller);
    reliable |= ctrl->get_reliable();
    channel = ctrl->get_channel();
    if (BOOST_UNLIKELY(channel >= kChannelCount))
      return;
  }

  auto buf = std::make_shared<boost::asio::streambuf>();
  std::ostream os(buf.get());

  auto& converter = converters_[channel];
  if (reliable && converter) {
    std::string encrypted_data;
    converter->handle_ostream_data(*str_data.get(), encrypted_data);
    os.write(encrypted_data.data(), encrypted_data.size());
    // send_count = encrypted_data.size();
  }
  else {
    os.write(str_data->data(), str_data->size());
    // send_count = str_data->size();
  }
  service_->async_write(buf, reliable, channel);
}

bool rpc_channel::handle_data(
    const char* data, std::size_t size, bool reliable, unsigned char channel) {
  if (BOOST_UNLIKELY(channel >= kChannelCount))
    return false;

  auto input_size = size;
  auto left_size = size;
  std::string decrypted_data;
  auto& converter = converters_[channel];
  if (converter) {
    converter->handle_istream_data(std::string(data, size), decrypted_data);
    data = const_cast<const char*>(decrypted_data.data());
    input_size = decrypted_data.size();
    left_size = input_size;
  }
  std::size_t total_consume_size = 0;
  while (input_size > total_consume_size) {
    auto [result, consume_size] = request_parsers_[channel].parse(
        requests_[channel], static_cast<const void*>(data), left_size);
    total_consume_size += consume_size;
    data += consume_size;
    left_size -= consume_size;
    if (result) {
      auto succ = on_request(channel);
      requests_[channel].reset();
      if (!succ)
        return false;
      continue;
    }
    else {
      return false;
    }
  }
  return true;
}

void rpc_channel::set_recv_limit(std::size_t limit) {
  for (auto i = 0u; i < kChannelCount; ++i)
    request_parsers_[i].set_recv_limit(limit);
}

void rpc_channel::enable_compressor(bool enabled, unsigned char channel) {
  if (BOOST_UNLIKELY(channel >= kChannelCount))
    return;

  auto& converter = converters_[channel];
  if (!converter)
    converter.reset(new rpc_converter());
  if (enabled) {
    converter->set_compressor(
        std::make_shared<nyx::compressor::zlib_compressor>(wbits_, memlevel_));
  }
  else {
    converter->reset_compressor();
  }
}

void rpc_channel::enable_encrypter(
    const std::string& key, unsigned char channel) {
  if (BOOST_UNLIKELY(channel >= kChannelCount))
    return;

  auto& converter = converters_[channel];
  if (!converter)
    converter.reset(new rpc_converter());
  if (key != "") {
    converter->set_crypter(
        std::make_shared<nyx::crypter::rc4_crypter>(key),
        std::make_shared<nyx::crypter::rc4_crypter>(key));
  }
  else {
    converter->reset_crypter();
  }
}

void rpc_channel::call_traverse(const rpc_traverse_msg_ptr& msg) {
  if (!msg || msg->empty())
    return;

  auto buf = std::make_shared<boost::asio::streambuf>();
  std::ostream os(buf.get());
  auto& converter = converters_[kDefaultChannelId];
  if (converter) {
    std::string encrypted_data;
    converter->handle_ostream_data(*(msg->get_msg().get()), encrypted_data);
    os.write(encrypted_data.data(), encrypted_data.size());
  }
  else {
    os.write(msg->get_msg()->data(), msg->get_msg()->size());
  }
  service_->async_write(buf);
}

}}
