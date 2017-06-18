// Copyright (c) 2017 ASMlover. All rights reserved.
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
	"flag"
	"fmt"
	"os"
)

func fill_buffer(buf []byte, buflen int, fill byte) {
	for i := 0; i < buflen; i++ {
		buf[i] = fill
	}
}

func cover_usage() {
	fmt.Printf("Cover - cover the disk\n\n")
	fmt.Printf("Usage:\n")
	fmt.Printf("\t--driver - setting the target disk driver(D:/E:, ect...)\n")
	fmt.Printf("\t--fill   - setting the buffer fill value(0/1/...)\n")
	fmt.Printf("\t--buflen - setting the length of buffer\n")
}

func main() {
	disk_driver := flag.String("driver", "E:", "Target disk driver")
	fill := flag.Int("fill", 0, "Buffer fill byte")
	buflen := flag.Int("buflen", 4 * 1024 * 1024, "Default buffer length")
	flag.Parse()

	if flag.NFlag() <= 1 {
		cover_usage()
		return
	}

	cover_fname := *disk_driver + "/COVER.BIN"
	fmt.Printf("cover_fname: %s\n", cover_fname)

	f, _ := os.OpenFile(cover_fname,
		os.O_CREATE | os.O_WRONLY | os.O_APPEND, 0644)
	defer f.Close()

	buf := make([]byte, *buflen)
	fill_buffer(buf, *buflen, uint8(*fill))
	count := 0
	for true {
		n, err := f.Write(buf)
		if err != nil {
			os.Exit(0)
		}
		count++
		fmt.Printf("[%d] Wrote %d bytes\n", count, n)
		f.Sync()
	}
}
