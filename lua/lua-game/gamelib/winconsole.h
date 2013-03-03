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
 *    notice, this list of conditions and the following disclaimer in
 *  * Redistributions in binary form must reproduce the above copyright
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
#ifndef __WIN_CONSOLE_HEADER_H__
#define __WIN_CONSOLE_HEADER_H__ 

#include <windows.h>
#include <string>
#include <list>

typedef struct clua_s clua_t;

class WinConsole {
  static volatile bool s_isWinActive_;
  static volatile HWND s_hWnd_;
  static volatile HWND s_hEditCtrl_;
  static char s_cmdBuf_[4096];

  std::list<std::string> strList_;
  HINSTANCE hInst_;

  clua_t* clua_;

  int scrollY_;
  int textHeight_;
public:
  static HWND startConsole(HINSTANCE hInst, clua_t* clua);
  static void stopConsole(void);
  static void write(const char* string);

  static LRESULT CALLBACK wndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp);
  static LRESULT CALLBACK editProc(HWND h, UINT msg, WPARAM wp, LPARAM lp);

  bool consoleReady(void) const 
  {
    return (NULL != s_hWnd_);
  }
private:
  WinConsole(void);
  virtual ~WinConsole(void);

  void init(HINSTANCE hInst);
  void resizeControls(void);
  void adjustScrollBar(void);
  void paint(HDC hDC);
};


extern WinConsole* g_wConsole;

#endif  /* __WIN_CONSOLE_HEADER_H__ */
