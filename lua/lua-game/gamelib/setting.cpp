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
#include <windows.h>
#include "setting.h"


#define REG_STRING_LEN_DEF  (256)


Setting::Setting(void)
{
}

Setting::~Setting(void)
{
}

void 
Setting::init(const char* baseKey)
{
  baseKey_ = "Software\\LuaGame\\";
  baseKey_ += baseKey;
}

int 
Setting::getInteger(const char* key, int defVal)
{
  int val = defVal;

  if (!regRead(key, &val, sizeof(DWORD), REG_DWORD))
    val = defVal;

  return val;
}

std::string
Setting::getString(const char* key, const char* defVal)
{
  static char buffer[REG_STRING_LEN_DEF];

  if (regRead(key, buffer, sizeof(buffer), REG_SZ))
    return buffer;

  return defVal;
}

void 
Setting::setInteger(const char* key, int value)
{
  regWrite(key, &value, sizeof(DWORD), REG_DWORD);
}

void 
Setting::setString(const char* key, const char* value)
{
  regWrite(key, (void*)value, REG_STRING_LEN_DEF, REG_SZ);
}



void 
Setting::regWrite(const char* key, void* buffer, int size, int type)
{
  HKEY hKey;
  DWORD success, disposition;

  success = RegCreateKeyEx(HKEY_CURRENT_USER, baseKey_.c_str(), 
      0, "LuaGame", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
      NULL, &hKey, &disposition);
  if (ERROR_SUCCESS == success) {
    success = RegSetValueEx(hKey, key, 0, type, (const BYTE*)buffer, size);
    RegCloseKey(hKey);
  }
}

bool 
Setting::regRead(const char* key, void* buffer, int size, int type)
{
  HKEY hKey;
  DWORD success, disposition;

  success = RegCreateKeyEx(HKEY_CURRENT_USER, baseKey_.c_str(), 
      0, "LuaGame", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
      NULL, &hKey, &disposition);
  if (ERROR_SUCCESS == success) {
    success = RegQueryValueEx(hKey, key, 0, 
        (DWORD*)&type, (BYTE*)buffer, (DWORD*)&size);
    RegCloseKey(hKey);
  }

  return (ERROR_SUCCESS == success);
}
