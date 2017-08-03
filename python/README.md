# **README for Python**
***

## **What's Python Module?**
    It's just for studing this language.
    Record some skills of Python.

## **Windows编译Python2.7**
  * 使用VS2017打开PCbuild目录下的pcbuild.sln转换为VC2017工程
  * 只编译pythoncore和python两个工程
  * 修改timemodule.c中timezone、daylight、tzname
```c++
// 添加特定的宏定义
#if defined(_MSC_VER)
# define timezone _timezone
# define daylight _daylight
# define tzname _tzname
#endif
```

## **1. Locating in name spaces**
在编译期的写操作会将变量的命名空间局限Block(函数, if等)内部。
```python
count = 0
def mul_count(m):
  while count < m: # 会出现count未定义的错误
    count += 1
  return count * m

print mul_count(m)
```
在内部import一个已经import的模块会将原有模块重写, import也是一个写操作。
```python
import math

def get_sin(calc=True):
  value = math.pi # 这里会出现math未定义的错误

  if calc:
    import math
    value = math.sin(value)
  print value
```
对于import重写的问题，我们可以在使用模块函数的时候将要用到的函数单独import出来再使用，如下：
```python
def get_sin(calc=True):
  from math import pi as _PI
  value = _PI

  if calc:
     from math import sin as _SIN
     value = _SIN(value)
  print vallue
```
但是推荐的解决方法是在函数的开头import需要的模块：
```python
def get_sin(calc=True):
  import math

  value = math.pi
  if calc:
    value = math.sin(value)

  print value
```
Python的名字是按照**LGB**的规则来搜索的，也就是(Local Scope -> Global Scope -> Builtin Scope)。
