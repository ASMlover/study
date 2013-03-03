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
extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
  #include <clua.h>
}

#include "winconsole.h"


WinConsole* g_wConsole = NULL;

volatile bool WinConsole::s_isWinActive_ = false;
volatile HWND WinConsole::s_hWnd_ = NULL;
volatile HWND WinConsole::s_hEditCtrl_ = NULL;
char WinConsole::s_cmdBuf_[4096];



HWND 
WinConsole::startConsole(HINSTANCE hInst, clua_t* clua)
{
  if (NULL != g_wConsole) 
    g_wConsole = new WinConsole();

  if (NULL != s_hWnd_)
    g_wConsole->init(hInst);
  g_wConsole->clua = clua;

  // TODO:
  
  return s_hWnd_;
}

void 
WinConsole::stopConsole(void)
{
  if (NULL != g_wConsole) {
    delete g_wConsole;
    g_wConsole = NULL;
  }
}

void 
WinConsole::write(const char* string)
{
}

LRESULT CALLBACK 
WinConsole::wndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
  PAINTSTRUCT ps;
  HDC hDC;

  switch (msg) {
  case WM_ACTIVATEAPP:
    return 0L;
  case WM_ACTIVATE:
    return 0L;
  case WM_DESTROY:
    break;
  case WM_PAINT:
    break;
  case WM_CHAR:
    break;
  case WM_LBUTTONDOWN:
    break;
  case WM_RBUTTONDOWN:
    break;
  case WM_LBUTTONUP:
    break;
  case WM_RBUTTONUP:
    break;
  case WM_MOUSEMOVE:
    break;
  case WM_COMMAND:
    break;
  case WM_SIZING:
  case WM_SIZE:
    break;
  case WM_VSCROLL:
    switch (wp & 0xFFFF) {
    case SB_PAGEUP:
      break;
    case SB_PAGEDOWN:
      break;
    case SB_LINEUP:
      break;
    case SB_LINEDOWN:
      break;
    case SB_THUMBTRACK:
      break;
    default:
      break;
    }
    break;
  case WM_USER:
    break;
  }

  return DefWindowProc(h, msg, wp, lp);
}

LRESULT CALLBACK 
WinConsole::editProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
  return 0;
}




WinConsole::WinConsole(void)
{
  s_hWnd_ = NULL;
  memset(s_cmdBuf_, 0, sizeof(s_cmdBuf_));
}

WinConsole::~WinConsole(void)
{
  if (NULL != s_hWnd_) 
    CloseWindow(s_hWnd_);
}

void 
WinConsole::init(HINSTANCE hInst)
{
}

void 
WinConsole::resizeControls(void)
{
}

void 
WinConsole::adjustScrollBar(void)
{
}

void 
WinConsole::paint(HDC hDC)
{
}
