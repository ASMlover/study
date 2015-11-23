# **Python奇技淫巧**
***

<h2 id="content">Content</h2>
  * [1. 显示有限的接口到外部](#1)
  * [2. with关键字](#2)
  * [3. filter的用法](#3)
  * [4. 一行作为判断(类C语言三目运算)](#4)
  * [5. 装饰器之单例](#5)
  * [6. staticmethod装饰器](#6)
  * [7. property装饰器](#7)
  * [8. iter魔法](#8)
  * [9. 神奇的partial](#9)
  * [10. eval](#10)

<h2 id="1">1. 显示有限的接口到外部</h2>
  当发布Python第三方package的时候，不希望代码中所有的函数或class可以被外部import，这个时候可以在`__init__.py`中添加`__all__`属性，该list中填写可以import的类或函数名，这样就可以起到限制import的作用，放置外部import其他函数或类。
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

from client import ClientEntity
from server import ServerEntity
from utils import LogFormatter

__all__ = ['ClientEntity', 'ServerEntity', 'LogFormatter']
```
[Back](#content)

<h2 id="2">2. with关键字</h2>
  with语句需要支持上下文管理协议的对象，上下文管理协议包含`__enter__`和`__exit__`。with语句建立运行时上下文需要通过这两个方法执行进入和退出操作。上下文表达式是跟在with之后的表达式，该表达式返回一个上下文管理对象。
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

with open('test.txt', 'r') as file_handler:
  for line in file_handler:
    print line
```
  我们可以自定义支持上下文管理协议的类，在类中实现`__enter__`和`__exit__`方法：
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

class MyClass(object):
  def __init__(self):
    print 'MyClass.__init__'

  def __enter__(self):
    print 'MyClass.__enter__'
    return self

  def __exit__(self, exc_type, exc_value, exc_traceback):
    print 'MyClass.__exit__'
    if exc_traceback is None:
      print 'MyClass.__exit__ without exception'
      return True
    else:
      print 'MyClass.__exit__ exception'
      return False

def test_with():
  with MyClass() as my_class:
    print 'running my_class'
  with MyClass() as my_class:
    print 'running before exception'
    raise Exception
    print 'running after exception'

if __name__ == '__main__':
  test_with()
```
  根据执行结果可以看见先执行`__enter__`方法，然后执行with之内的逻辑，最后执行`__exit__`做退出处理，即使出现异常也可以正常退出。
[Back](#content)


<h2 id="3">3. filter的用法</h2>
  相对filter而言，map和reduce的使用频率更频繁一些，filter是按照某种规则过滤一些元素。
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

my_list = [1, 2, 3, 4, 5, 6]
print filter(lambda x: x % 2 != 0, my_list) # 过滤所有偶数
```
[Back](#content)

<h2 id="4">4. 一行作为判断(类C语言三目运算)</h2>
  当条件满足时，返回的为等号后面的变量，否则返回else后面的语句。
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

my_list = [1, 2, 3]
new_list = my_list[0] if my_list is not None else None
print new_list
```
[Back](#content)

<h2 id="5">5. 装饰器之单例</h2>
  使用装饰器实现简单的单例。
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

def singleton(cls):
  _inst = {}
  def _singleton(*args, **kwargs):
    if cls not in _inst:
      _inst[cls] = cls(*args, **kwargs)
    return _inst[cls]
  return _singleton

@singleton
class MyClass(object):
  pass

if __name__ == '__main__':
  c1 = MyClass()
  c2 = MyClass()
  print c1, c2
```
[Back](#content)

<h2 id="6">6. staticmethod装饰器</h2>
  类中两种常用的装饰，首先区分以下：
  * 普通成员函数，其中第一个隐式参数为对象；
  * classmethod装饰器，类方法(给人感觉类似与OC中的类方法)，其中第一个隐式参数为类；
  * staticmethod装饰器，没有任何隐式参数，python中的静态方法类似于C++中的静态方法；
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

class MyClass(object):
  def normal_fun(self, x):
    print 'executing normal_fun(%s, %s)' % (self, x)

  @classmethod
  def class_fun(cls, x):
    print 'executing class_fun(%s, %s)' % (cls, x)

  @staticmethod
  def static_fun(x):
    print 'executing static_fun(%s)' % x

if __name__ == '__main__':
  c = MyClass()
  c.normal_fun('Hello, world')
  c.class_fun('Hello, world')
  MyClass.class_fun('Hello, world')
  c.static_fun('Hello, world')
  MyClass.static_fun('Hello, world')
```
[Back](#content)

<h2 id="7">7. property装饰器</h2>
  将property与装饰器结合可以实现属性私有化(更简单安全实现get和set方法)
```python
# python的内建函数
property(fget=None, fset=None, fdel=None, doc=None)
```
  fget是获取属性值的函数，fset是设置属性值的函数，fdel是删除属性值的函数，doc是一个字符串，这些参数都是可选的。property有3ge方法getter()，setter()和delete()来指定fget，fset和fdel。
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

class Student(object):
  @property # 相当于property.getter(score)或property(score)
  def score(self):
    return self._score

  @score.setter # 相当于score = property.setter(score)
  def score(self, value):
    if not isinstance(value, int):
      raise ValueError('score must be an integer')
    if value < 0 or value > 100:
      raise ValueError('score must between 0 ~ 100')
    self._score = value
```
[Back](#content)

<h2 id="8">8. iter魔法</h2>
  * 通过yield和__iter__结合，可以把一个对象变成可迭代对象
  * 通过重写__str__，可按自己的需求打印对象
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

class MyClass(object):
  def __init__(self):
    self._my_value = [1, 2, 3, 4, 5]

  def read(self):
    for v in xrange(len(self._my_value)):
      yield v

  def __iter__(self):
    return self.read()

  def __str__(self):
    return ','.join(map(str, self._my_value))

  __repr__ = __str__

if __name__ == '__main__':
  c = MyClass()
  for v in c:
    print v
  print c
```
[Back](#content)

<h2 id="9">9. 神奇的partial</h2>
  类似C++中的仿函数(函数对象)，类似partial的运行方式如下：
```python
#!/usr/bin/python
# -*- coding: utf-8 -*-

def partial(fun, *p_args):
  def wrapper(*extra_args):
    args = list(p_args)
    args.extend(extra_args)
    return fun(*args)

  return wrapper
```
  利用闭包的特性绑定一些函数参数，返回一个可调用的变量，直到真正的调用执行。
```python
#!/usr/bin/python
# -*- coding: utf-8 -*-

from functools import partial

def add_fun(a, b):
  return a + b

if __name__ == '__main__':
  fun = partial(add_fun, 2)
  print fun(3) # 实现执行的就是add_fun(2, 3)
```
[Back](#content)

<h2 id="10">10. eval</h2>
  是内嵌的python解释器，可解释字符串对于的代码并执行将执行的结果返回。
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

def action_fun():
  print 'action_fun'
  return True

action = {"cond_val": 5, "do_action": action_fun}

if __name__ == '__main__':
  r = eval('cond_val > 3 and do_action()', action)
  print r
```
[Back](#content)

<h2 id="11">11. exec</h2>
  * exec会忽略返回值，总是返回None，eval会返回执行代码或语句的返回值
  * exec和eval在执行代码时，除了返回值其他都相同
  * 传入字符串时，会使用compile(source, "<string>", mode)编译字节码，mode的取值为exec和eval
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

def action_fun():
  print 'action_fun'
  return True

action = {'cond_val': 5, 'do_action': action_fun}

if __name__ == '__main__':
  r = exec('cond_val > 3 and do_action()', action)
  print r
```
[Back](#content)
