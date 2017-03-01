# **README for Python**
***

## **What's Python Module?**
    It's just for studing this language.
    Record some skills of Python.

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
