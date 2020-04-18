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
	"fmt"
	"net"
	"os"
)

const (
	WAIT_MSG = 1
	READ_MSG = 2
)

func onServe(conn net.Conn) {
	defer conn.Close()

	if _, err := conn.Write([]byte("*")); err != nil {
		return
	}

	status := WAIT_MSG
	for {
		var rbuf [1024]byte
		n, err := conn.Read(rbuf[0:])
		if err != nil {
			return
		}

		var wbuf []byte
		for i := 0; i < n; i++ {
			switch status {
			case WAIT_MSG:
				if rbuf[i] == '^' {
					status = READ_MSG
				}
			case READ_MSG:
				if rbuf[i] == '$' {
					status = WAIT_MSG
				} else {
					wbuf = append(wbuf, rbuf[i]+1)
				}
			}
		}
		if _, err := conn.Write(wbuf[0:]); err != nil {
			return
		}
	}
}

func main() {
	acceptor, err := net.Listen("tcp", ":5555")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Fatal error: %s", err.Error())
		os.Exit(1)
	}

	for {
		conn, err := acceptor.Accept()
		if err != nil {
			continue
		}
		go onServe(conn)
	}
}
