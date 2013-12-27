//! Copyright (c) 2013 ASMlover. All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!  * Redistributions of source code must retain the above copyright
//!    notice, this list ofconditions and the following disclaimer.
//!
//!  * Redistributions in binary form must reproduce the above copyright
//!    notice, this list of conditions and the following disclaimer in
//!    the documentation and/or other materialsprovided with the
//!    distribution.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//! FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//! COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//! INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//! BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//! CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//! LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//! ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//! POSSIBILITY OF SUCH DAMAGE.
#include "../el_net_internal.h"
#include "../el_time.h"
#include "../el_net.h"
#include "../el_socket.h"
#include "../el_connector.h"
#include "../el_network_handler.h"



class ConnectorHandler : public el::EventHandler {
public:
  virtual bool AcceptEvent(el::Connector* conn)
  {
    el::ColorFprintf(stdout, el::kColorTypeGreen, 
        "accept connector [%d]\n", conn->fd());
    return true;
  }

  virtual void CloseEvent(el::Connector* conn)
  {
    el::ColorFprintf(stdout, el::kColorTypeGreen, 
        "connector [%d] closed\n", conn->fd());
  }

  virtual bool ReadEvent(el::Connector* conn)
  {
    char buf[128] = {0};
    if (el::kNetTypeError != conn->Read(sizeof(buf), buf))
      conn->Write(buf, strlen(buf));

    return true;
  }
};



void 
ServerMain(const char* ip = "0.0.0.0", uint16_t port = 5555)
{
  ConnectorHandler ch;

  el::NetworkHandler network;
  network.Attach(&ch);

  if (!network.Init())
    return;

  if (!network.Listen(ip, port)) {
    el::ColorFprintf(stderr, el::kColorTypeRed, 
        "starting server <%s, %d> failed ...\n", ip, port);
    return;
  }
  else {
    el::ColorFprintf(stdout, el::kColorTypeGreen, 
        "starting server <%s, %d> success ...\n", ip, port);
  }

  while (true)
    el::Sleep(100);

  network.Destroy();
}
