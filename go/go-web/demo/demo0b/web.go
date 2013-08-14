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
  "io"
  "fmt"
  "html/template"
  "log"
  "net/http"
  "strings"
  "time"
  "crypto/md5"
  "strconv"
  "os"
)



func sayHello(w http.ResponseWriter, r *http.Request) {
  r.ParseForm() //! 解析url传递的参数, POST则解析响应包的主体, 该函数必须有

  fmt.Println(r.Form)
  fmt.Println("path", r.URL.Path)
  fmt.Println("scheme", r.URL.Scheme)
  fmt.Println(r.Form["url_long"])
  for k, v := range r.Form {
    fmt.Println("key: ", k)
    fmt.Println("val: ", strings.Join(v, ""))
  }

  fmt.Fprintf(w, "Hello, world!")
}


func login(w http.ResponseWriter, r *http.Request) {
  fmt.Println("method: ", r.Method)
  if r.Method == "GET" {
    curtime := time.Now().Unix()
    h := md5.New()
    io.WriteString(h, strconv.FormatInt(curtime, 10))
    token := fmt.Sprintf("%x", h.Sum(nil))

    t, _ := template.ParseFiles("login.gtpl")
    t.Execute(w, token)
  } else {
    r.ParseForm()
    token := r.Form.Get("token")
    if token != "" {
    } else {
    }

    fmt.Println("username length:", len(r.Form["username"][0]))

    fmt.Println("username:",
      template.HTMLEscapeString(r.Form.Get("username")))
    fmt.Println("password:",
      template.HTMLEscapeString(r.Form.Get("password")))
    template.HTMLEscape(w, []byte(r.Form.Get("username")))
  }
}


func upload(w http.ResponseWriter, r *http.Request) {
  fmt.Println("method:", r.Method)
  if r.Method == "GET" {
    curtime := time.Now().Unix()
    h := md5.New()
    io.WriteString(h, strconv.FormatInt(curtime, 10))
    token := fmt.Sprintf("%x", h.Sum(nil))

    t, _ := template.ParseFiles("upload.gtpl")
    t.Execute(w, token)
  } else {
    r.ParseMultipartForm(32 << 20)
    file, handler, err := r.FormFile("uploadfile")
    if err != nil {
      fmt.Println(err)
      return
    }

    defer file.Close()
    fmt.Fprintf(w, "%v", handler.Header)
    err1 := os.Mkdir("test", 0666)
    if err1 != nil {
      fmt.Println(err1)
    }

    f, err := os.OpenFile("./test/" + handler.Filename,
      os.O_WRONLY | os.O_CREATE, 0666)
    if err != nil {
      fmt.Println(err)
      return
    }

    defer f.Close()
    io.Copy(f, file)
  }
}



func main() {
  http.HandleFunc("/", sayHello)
  http.HandleFunc("/login", login)
  http.HandleFunc("/upload", upload)
  err := http.ListenAndServe(":8080", nil)
  if err != nil {
    log.Fatal("ListenAndServe: ", err)
  }
}
