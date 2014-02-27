# **VS Debug**
***



## **Summary**
 * 记录使用VS进行调试的一些技巧



## **VS Watch窗口**
 * 伪变量
  - @err:           当前断点处GetLastError()的值
  - $handles:       当前应用句柄数量
  - $clk:           the time in clock cycles
  - $vframe:        address of the current stack frame
  - @reg || $reg:   查看寄存器内容
  - @eax:           最近一次函数返回值, 64位系统使用@rax
  - $tid:           当前线程ID
  - $pid:           当前进程ID
  - $user:          当前OS用户名字
  - $cmdline:       进程命令行
 * 数值格式化
  - 0x10,wm:        WM_CLOSE
  - 0x00400000,wc:  WS_OVERLAPPEDWINDOW
  - @err,hr:        0x00000002系统找不到指定的文件
  - 进制转换  
        (1). 123,i:   有符号十进制
        (2). 123,d:   有符号十进制
        (3). 123,u:   无符号十进制
        (4). 123,o:   无符号八进制
        (5). 123,x:   小写十六进制
        (6). 123,X:   大写十六进制
 * 内存查看
  - ptr:            当成字符串查看(含地址)
  - ptr,s:          当成字符串查看(不含地址)
  - ptr,8:          当成BYTE[8]查看 
  - ptr,mb:         当成BYTE[16]查看 
  - ptr,mw:         当成WORD[8]查看 
  - ptr,md:         当成DWORD[4]查看 
  - ptr,mq:         当成ULONGLONG[2]查看
