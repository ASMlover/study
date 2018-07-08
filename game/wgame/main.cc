// Copyright (c) 2018 ASMlover. All rights reserved.
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
#include <Windows.h>
#include <tchar.h>

LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
  switch (msg) {
  case WM_CREATE:
    {
      RECT rc{};
      GetClientRect(hWnd, &rc);
      auto x = GetSystemMetrics(SM_CXSCREEN) / 2 - (rc.right - rc.left) / 2;
      auto y = GetSystemMetrics(SM_CYSCREEN) / 2 - (rc.bottom - rc.top) / 2;
      SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
    } break;
  case WM_KEYDOWN:
    switch (wp) {
    case VK_ESCAPE:
      MessageBox(hWnd, TEXT("ESC down"), TEXT("Keyboard"), MB_OK);
      PostMessage(hWnd, WM_CLOSE, 0, 0);
      break;
    }
    return 0;
  case WM_CLOSE:
    DestroyWindow(hWnd);
    return 0;
  case WM_RBUTTONDOWN:
    MessageBox(hWnd, TEXT("Right button down"), TEXT("Mouse"), MB_OK);
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }

  return DefWindowProc(hWnd, msg, wp, lp);
}

static BOOL initWindow(HINSTANCE hInst, int cmdShow) {
  WNDCLASS wc{};
  wc.lpfnWndProc = static_cast<WNDPROC>(wndProc);
  wc.hInstance = hInst;
  wc.hbrBackground = CreateSolidBrush(RGB(100, 0, 0));
  wc.lpszClassName = TEXT("Hello");
  RegisterClass(&wc);

  auto hWnd = CreateWindow(
      TEXT("Hello"), TEXT("Hello, world"),
      WS_POPUP | WS_MAXIMIZE, 0, 0,
      GetSystemMetrics(SM_CXSCREEN) / 2,
      GetSystemMetrics(SM_CYSCREEN) / 2,
      NULL, NULL, hInst, NULL);
  if (!hWnd)
    return FALSE;
  ShowWindow(hWnd, cmdShow);
  UpdateWindow(hWnd);
  return TRUE;
}

int CALLBACK _tWinMain(
    HINSTANCE hInst, HINSTANCE hPrev, LPTSTR cmdLine, int cmdShow) {
  if (!initWindow(hInst, cmdShow))
    return 0;

  MSG msg;
  for (;;) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT)
        break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  return static_cast<int>(msg.wParam);
}
