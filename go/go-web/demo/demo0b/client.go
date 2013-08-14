/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
package main
import (
  "fmt"
  "bytes"
  "io"
  "io/ioutil"
  "mime/multipart"
  "net/http"
  "os"
)



func postFile(fname string, targeturl string) error {
  bodyBuf := &bytes.Buffer{}
  bodyWriter := multipart.NewWriter(bodyBuf)

  fileWriter, err := bodyWriter.CreateFormFile("uploadfile", fname)
  if err != nil {
    fmt.Println(err)
    return err
  }

  fh, err := os.Open(fname)
  if err != nil {
    fmt.Println(err)
    return err
  }

  _, err = io.Copy(fileWriter, fh)
  if err != nil {
    return err
  }

  contentType := bodyWriter.FormDataContentType()
  bodyWriter.Close()

  resp, err := http.Post(targeturl, contentType, bodyBuf)
  if err != nil {
    return err
  }

  defer resp.Body.Close()
  respBody, err := ioutil.ReadAll(resp.Body)
  if err != nil {
    return err
  }

  fmt.Println(resp.Status)
  fmt.Println(string(respBody))

  return nil
}


func main() {
  targeturl := "http://localhost:8080/upload"
  fname := "login.gtpl"

  postFile(fname, targeturl)
}
