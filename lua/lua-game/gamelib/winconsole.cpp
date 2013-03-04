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
  if (NULL != g_wConsole && NULL != s_hWnd_) {
    int str_len = (int)strlen(string);
    char* buf = (char*)malloc(str_len + 1);
    int index = 0;

    for (int i = 0; i < (int)str_len; ++i) {
      if (10 != string[i])
        buf[index++] = string[i];
      else {
        buf[index] = 0;
        g_wConsole->strList_.push_front(buf);
        index = 0;
      }
    }

    if (index > 0) {
      buf[index] = 0;
      g_wConsole->strList_.push_front(buf);
    }
    InvalidateRect(s_hWnd_, NULL, TRUE);
    free(buf);
  }

  g_wConsole->adjustScrollBar();
}

LRESULT CALLBACK 
WinConsole::wndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
  PAINTSTRUCT ps;
  HDC hDC;

  switch (msg) {
  case WM_ACTIVATEAPP:
    s_isWinActive_ = (wp != 0);
    return 0L;
  case WM_ACTIVATE:
    s_isWinActive_ = ((wp == WM_ACTIVATE) || (wp == WA_CLICKACTIVE));
    return 0L;
  case WM_DESTROY:
    s_isWinActive_ = false;
    s_hWnd_ = NULL;
    break;
  case WM_PAINT:
    hDC = BeginPaint(s_hWnd_, &ps);
    g_wConsole->paint(hDC);
    EndPaint(s_hWnd_, &ps);
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
    g_wConsole->resizeControls();
    break;
  case WM_SETCURSOR:
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    ShowCursor(TRUE);
    break;
  case WM_VSCROLL:
    switch (wp & 0xFFFF) {
    case SB_PAGEUP:
      g_wConsole->scrollY_ = 
        min((int)(g_wConsole->scrollY_ + g_wConsole->textHeight_), 
          (int)(g_wConsole->strList_.size() - g_wConsole->textHeight_) + 1);
      break;
    case SB_PAGEDOWN:
      break;
    case SB_LINEUP:
      g_wConsole->scrollY_ = 
        min((int)(g_wConsole->scrollY_ + 1), 
          (int)(g_wConsole->strList_.size() - g_wConsole->textHeight_) + 1);
      break;
    case SB_LINEDOWN:
      g_wConsole->scrollY_ = max(g_wConsole->scrollY_ - 1, 0);
      break;
    case SB_THUMBTRACK:
      break;
    default:
      break;
    }

    {
      SCROLLINFO si;
      si.cbSize = sizeof(si);
      si.fMask  = SIF_POS;
      si.nPos   = g_wConsole->strList_.size() - g_wConsole->scrollY_;
      SetScrollInfo(h, SB_VERT, &si, TRUE);
    } 
    InvalidateRect(s_hWnd_, NULL, TRUE);

    break;
  case WM_USER:
    write(s_cmdBuf_);
    if (0 != luaL_loadbuffer(clua_get_context(g_wConsole->clua), 
          s_cmdBuf_, strlen(s_cmdBuf_), NULL))
      write("Error loading command\n");
    if (0 != lua_pcall(clua_get_context(g_wConsole->clua), 
          0, LUA_MULTRET, 0))
    { 
      write("Error in command\n");
      write(luaL_checkstring(clua_get_context(g_wConsole->clua), -1));
    }
    memset(s_cmdBuf_, 0, sizeof(s_cmdBuf_));

    break;
  }

  return DefWindowProc(h, msg, wp, lp);
}

LRESULT CALLBACK 
WinConsole::editProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
  switch (msg) {
  case WM_CHAR:
    if ((TCHAR)wp == VK_RETURN) {
      LONG sizeOfString = SendMessage(h, WM_GETTEXTLENGTH, 0, 0);
      SendMessage(h, WM_GETTEXT, sizeOfString + 1, (LPARAM)s_cmdBuf_);
      SendMessage(s_hWnd_, WM_USER, 0, sizeOfString);
      SendMessage(h, WM_SETTEXT, 0, (LONG)TEXT(""));

      return 1;
    }
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
