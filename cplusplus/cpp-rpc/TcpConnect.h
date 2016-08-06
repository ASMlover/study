// Copyright (c) 2016 ASMlover. All rights reserved.
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
#ifndef __TCP_CONNECT_HEADER_H__
#define __TCP_CONNECT_HEADER_H__

#include <memory>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <google/protobuf/service.h>

using namespace boost::asio;
typedef std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;

class TcpConnection : public google::protobuf::RpcChannel {
  SocketPtr socket_;
  std::vector<google::protobuf::Service*> services_;
public:
  explicit TcpConnection(boost::asio::io_service& io);
  ~TcpConnection(void);

  void WriteMessage(const std::string& str);
  void WriteHandler(const boost::system::error_code& ec);
  void ReadHandler(const boost::system::error_code& ec,
      std::shared_ptr<std::vector<char> > str);
  void AddService(google::protobuf::Service* service);
  SocketPtr getSocket(void) const;
private:
  void DealRpcData(std::shared_ptr<std::vector<char> > str);
};

#endif  // __TCP_CONNECT_HEADER_H__
