# **Python奇技淫巧**
***

## **1. 显示有限的接口到外部**
  当发布Python第三方package的时候，不希望代码中所有的函数或class可以被外部import，这个时候可以在`__init__.py`中添加`__all__`属性，该list中填写可以import的类或函数名，这样就可以起到限制import的作用，放置外部import其他函数或类。
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

from client import ClientEntity
from server import ServerEntity
from utils import LogFormatter

__all__ = ['ClientEntity', 'ServerEntity', 'LogFormatter']
```

## **2. with关键字**
  with语句需要支持上下文管理协议的对象，上下文管理协议包含`__enter__`和`__exit__`。with语句建立运行时上下文需要通过这两个方法执行进入和退出操作。上下文表达式是跟在with之后的表达式，该表达式返回一个上下文管理对象。
```python
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
