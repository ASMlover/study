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
#include <WindowsX.h>
#include <tchar.h>
#include "ddutils.h"

HRESULT dd_reloadBitmap(IDirectDrawSurface7* dds, LPCTSTR bitmap) {
  auto bm = (HBITMAP)LoadImage(
      GetModuleHandle(NULL), bitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
  if (bm == NULL) {
    bm = (HBITMAP)LoadImage(NULL,
        bitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  }
  if (bm == NULL) {
    OutputDebugString(TEXT("HBITMAP handle is NULL\n"));
    return E_FAIL;
  }

  auto hr = dd_copyBitmap(dds, bm, 0, 0, 0, 0);
  if (hr != DD_OK)
    OutputDebugString(TEXT("dd_copyBitmap failed\n"));
  DeleteObject(bm);
  return hr;
}

HRESULT dd_copyBitmap(
    IDirectDrawSurface7* dds, HBITMAP hbm, int x, int y, int dx, int dy) {
  if (dds == NULL || hbm == NULL)
    return E_FAIL;

  dds->Restore();

  auto dcImage = CreateCompatibleDC(NULL);
  if (dcImage == NULL)
    OutputDebugString(TEXT("CreateCompatibleDC failed\n"));
  auto oldmap = SelectObject(dcImage, hbm);

  BITMAP bm;
  GetObject(hbm, sizeof(bm), &bm);
  dx = dx == 0 ? bm.bmWidth : dx;
  dy = dy == 0 ? bm.bmHeight : dy;

  DDSURFACEDESC2 ddsd{};
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
  dds->GetSurfaceDesc(&ddsd);

  HRESULT hr;
  HDC dc;
  if ((hr = dds->GetDC(&dc)) == DD_OK) {
    StretchBlt(dc,
        0, 0, ddsd.dwWidth, ddsd.dwHeight, dcImage, x, y, dx, dy, SRCCOPY);
    dds->ReleaseDC(dc);
  }
  SelectObject(dcImage, oldmap);
  DeleteDC(dcImage);

  return hr;
}

static DWORD dd_colorMatch(IDirectDrawSurface7* dds, COLORREF rgb) {
  HDC dc;
  COLORREF rgb_pixel;
  if (rgb != CLR_INVALID && dds->GetDC(&dc) == DD_OK) {
    rgb_pixel = GetPixel(dc, 0, 0);
    SetPixel(dc, 0, 0, rgb);
    dds->ReleaseDC(dc);
  }

  DDSURFACEDESC2 ddsd{};
  HRESULT hr;
  ddsd.dwSize = sizeof(ddsd);
  while ((hr = dds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING) {
  }

  DWORD dw = CLR_INVALID;
  if (hr == DD_OK) {
    dw = *(DWORD*)ddsd.lpSurface;
    dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;
    dds->Unlock(NULL);
  }

  if (rgb != CLR_INVALID && dds->GetDC(&dc) == DD_OK) {
    SetPixel(dc, 0, 0, rgb_pixel);
    dds->ReleaseDC(dc);
  }
  return dw;
}

HRESULT dd_setColorKey(IDirectDrawSurface7* dds, COLORREF rgb) {
  DDCOLORKEY ddck{};

  ddck.dwColorSpaceLowValue = dd_colorMatch(dds, rgb);
  ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
  return dds->SetColorKey(DDCKEY_SRCBLT, &ddck);
}
