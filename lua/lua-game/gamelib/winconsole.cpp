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


static WNDPROC s_EditProc;
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
  switch (msg) {
  case WM_CHAR:
    break;
  }
  return CallWindowProc(s_EditProc, h, msg, wp, lp);
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
  hInst_ = hInst;
  scrollY_ = 0;

  WNDCLASSEX wc = {0};

  wc.cbSize         = sizeof(wc);
  wc.style          = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc    = (WNDPROC)&WinConsole::wndProc;
  wc.hInstance      = hInst;
  wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszClassName  = TEXT("WinConsole");
  RegisterClassEx(&wc);

  s_hWnd_ = CreateWindowEx(0, 
      TEXT("WinConsole"), 
      TEXT("LUA WinConsole"), 
      WS_OVERLAPPEDWINDOW, 
      0,
      0, 
      640, 
      480, 
      NULL, 
      NULL, 
      hInst_, 
      NULL);
  ShowWindow(s_hWnd_, SW_SHOW);
  UpdateWindow(s_hWnd_);
  SetFocus(s_hWnd_);

  s_hEditCtrl_ = CreateWindowEx(0, 
      TEXT("Edit"), 
      TEXT(""), 
      ES_LEFT | WS_CHILD, 
      2, 
      404, 
      228, 
      16, 
      s_hWnd_, 
      (HMENU)0xa7, 
      hInst_, 
      NULL);
  s_EditProc = (WNDPROC)SetWindowLong(s_hEditCtrl_, 
      GWL_WNDPROC, (LONG)&WinConsole::editProc);
  g_wConsole->resizeControls();
}

void 
WinConsole::resizeControls(void)
{
  RECT rc;

  GetClientRect(s_hWnd_, &rc);
  textHeight_ = (rc.bottom - rc.top) / 16;

  SetWindowPos(s_hEditCtrl_, 
      HWND_TOP, 
      rc.left + 2, 
      rc.bottom - 18, 
      rc.right - rc.left - 4, 
      16, 
      SWP_NOZORDER);

  adjustScrollBar();
  InvalidateRect(s_hWnd_, NULL, TRUE);
}

void 
WinConsole::adjustScrollBar(void)
{
  SCROLLINFO si = {0};

  si.cbSize = sizeof(si);
  si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
  si.nMax   = strList_.size();
  si.nPage  = textHeight_;
  si.nPos   = strList_.size() - scrollY_;
  SetScrollInfo(s_hWnd_, SB_VERT, &si, NULL);
}

void 
WinConsole::paint(HDC hDC)
{
  SetTextColor(hDC, RGB(255, 255, 255));
  SetBkColor(hDC, RGB(0, 0, 0));

  RECT rc;
  GetClientRect(s_hWnd_, &rc);

  int x = 2;
  int y = rc.bottom - 40;

  std::list<std::string>::iterator it = strList_.begin();
  int skip = scrollY_;
  while (0 != skip) {
    ++it;
    --skip;
  }

  while (it != strList_.end()) {
    TextOut(hDC, x, y, it->c_str(), strlen(it->c_str()));
    y -= 16;
    ++it;
  }
}
