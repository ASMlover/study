# **Python奇技淫巧**
***

## **1. 显示有限的接口到外部**
    当发布Python第三方package的时候，不希望代码中所有的函数或class可以被外部import，这个时候可以在__init__.py中添加__all__属性，该list中填写可以import的类或函数名，这样就可以起到限制import的作用，放置外部import其他函数或类。
```python
#!/usr/bin/env python
# -*- coding: utf-8 -*-

from client import ClientEntity
from server import ServerEntity
from utils import LogFormatter

__all__ = ['ClientEntity', 'ServerEntity', 'LogFormatter']
```
