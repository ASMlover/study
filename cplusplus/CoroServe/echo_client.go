// Copyright (c) 2020 ASMlover. All rights reserved.
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

package main

import (
	"bytes"
	"fmt"
	"net"
	"os"
)

func onEcho(conn net.Conn) {
	var ack [64]byte
	if _, err := conn.Read(ack[0:]); err != nil {
		fmt.Fprintf(os.Stderr, "CLIENT: connect with server failed\n")
		return
	}

	wbuf := []byte("^abcdefg$hijklmn^opqrst$uvwxyz^000$")
	if _, err := conn.Write(wbuf[0:]); err != nil {
		return
	}
	fmt.Println("CLIENT: send:", string(wbuf))

	var rbuf []byte
	for {
		var buf [1024]byte
		n, err := conn.Read(buf[0:])
		if err != nil {
			return
		}

		rbuf = bytes.Join([][]byte{rbuf, buf[0:n]}, []byte(""))
		if bytes.Contains(rbuf, []byte("111")) {
			break
		}
	}
	fmt.Println("CLIENT: recv:", string(rbuf))

	conn.Close()
	fmt.Println("CLIENT: disconnecting ...")
}

func main() {
	conn, err := net.Dial("tcp", "127.0.0.1:5555")
	if err != nil {
		return
	}
	onEcho(conn)
}
