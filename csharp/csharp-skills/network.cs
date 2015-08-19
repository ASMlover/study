// Copyright (c) 2015 ASMlover. All rights reserved.
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
using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace network {
  class Program {
    static void Main(string[] args) {
      IPAddress addr = IPAddress.Parse("127.0.0.1");
      Socket client = new Socket(
          AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

      try {
        client.Connect(new IPEndPoint(addr, 5555));
        // connect success
      }
      catch {
        // connect failed
        client.Shutdown(SocketShutdown.Both);
        client.Close();
        return;
      }

      bool running = true;
      Thread t = new Thread(() => {
        byte[] recv_buffer = new byte[1024];
        while (running) {
          int recv_length = client.Receive(recv_buffer);
          // deal with receiving data
        }
      });
      t.Start();

      string send_buffer = "Hello, world!";
      client.Send(Encoding.ASCII.GetBytes(send_buffer));

      running = false;
      t.Join();
    }
  }
}
