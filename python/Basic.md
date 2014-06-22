# **Python基础知识**
***


## **1. 数据结构**
> ### **1.1 列表**
    * list.append(x)    追加新对象
    * list.extend(l)    在列表末尾追加另一个列表
    * list.insert(i,x)  将对象插入到列表中
    * list.remove(x)    从列表中移除x的第一个匹配选项
    * list.pop([i])     从列表中移除一个元素, 默认最后一个
    * list.index(x)     返回x的第一个匹配选项的索引
    * list.count(x)     统计某元素在列表中出现次数
    * list.sort()       对列表进行排序(会修改原列表),可传递一个比较函数
    * list.reverse()    反转列表
    * x = sorted(y)     对y排序, 返回排序结果, 不修改y
> ### **1.2 元组**
    就是一个不可改变的列表, 可以作为键
> ### **1.3 字符串**
    * str.find(sub[, start[, end]])
      在str中寻找子串sub, 返回sub所在位置最左端位置;
    * str.join(iterable)将字符串连接在一起
    * str.lower()       字符串小写
    * str.replace(old, new[, count])
      返回某字符串的所有匹配项均被替换之后得到的字符串
    * str.split([sep[, maxsplit]])
      根据sep将字符串分割成序列
    * str.strip([chars])
      去除左右两边的含有chars的字符, 默认是空格, 不包含中间
    * str.translate(table[, deletechars])
      替换字符串的某些部分, 要求一个替换后的字符表, 和一个被替换的字符表
      'read this short text'.translate(None, 'aeiou')
      'rd ths shrt txt'
> ### **1.4 字典**
    * 字典格式化
      book = {'bookname1' : 'author1', 'bookname2' : 'author2'}
      print '<<bookname1>> author is : %(bookname1)s' % book
    * dict.clear()      清除字典中所有项
    * dict.copy()       拷贝一个具相同键-值对的字典(浅拷贝)值是可变的才可改
    * dict.fromkeys(seq[, value])
      用给定的序列元素作为key创建一个字典, 值默认为None;
    * dict.get(key[, default])
      如果字典中有key, 返回key对应的值, 否则返回default(默认None);
    * dict.has_key(key) 检查字典中是否有给定的key;
    * dict.items()/dict.iteritems()
      items以列表形式返回字典的键值对(元组形式), 没有特殊的顺序;
      iteritems返回一个迭代器对象, 每次迭代为键值对(元组);
    * dict.keys()/dict.iterkeys()
      keys以列表形式返回字典中的键; iterkeys返回针对键的迭代器;
    * dict.pop(key[, default])
      获取key对应的值, 并将这个键-值对删除; 如果提供default, 没有key返回默认
    * dict.popitem()    弹出一个随机项
    * dict.setdefault(key[, default])
      如何key存在返回它的值, 如果不存在设置key的值为default, 默认为None;
    * dict.update([other])
      利用一个字典更新另一个字典, 新的项添加到dict, 旧的项被other替换;
    * dict.values()/dict.itervalues()
      values以列表形式返回字典中的值, itervalues返回字典中值的迭代器;



## **2. 控制结构**
    False, None, 0, '', (), [], {}
    标准值False和None, 所有类型的数字0(浮点数, 长整型和其他类型), 空序列(空
    字符串, 元组和列表), 空的字典为假, 其他一切都是真;
> ### **2.1 迭代**
    1) 并行迭代
        zip(seq1, seq2)
        seq1, seq2可以是长度不等的序列;
        zip合成以(seq1_item1, seq1_item2)元组组成的队列;
          names = ['name1', 'name2', 'name3']
          ages = [11, 22, 33]
          for name, age in zip(names, ages):
            print name, age 
    2) 编号迭代
        使用enumerate(seq), 返回索引和元素;
          for i, name in enumerate(names):
            print i, name 
    3) 翻转和排序迭代
        sorted方法和reserved方法, 返回原序列的副本, reserved返回一个迭代器 
          l = [23, 45, 7, 3, 34, 89, 9, 0, 121]
          x = sorted(l)
          print x
          x = reserved(l)
          [i*1 for i in x]
> ### **2.2 列表推导**
    是用其他列表创建新列表的方法;
        [x * x for x in range(10)]




## **3. 抽象**
> ### **3.1 函数**
    1) 搜集参数(用户可以传递任意多个参数, 其实时以元组的方式传递的; 或以键值
       对的方式传递任意多个参数, 是以字典方式实现的)
       func(*args)  元组
       func(**args) 字典 
        * map(func, seq[,seq,...])    对序列中的每个元素应用函数
        * filter(func, seq)           返回其函数为真的元素的列表
        * reduce(func,seq[,initial])  
          等同func(func(seq[0],seq[1],seq[2]),...)
        * sum(seq)                    返回seq中所有元素的和
        * apply(func[,args[,kwargs]]) 调用函数, 可以提供参数
> ### **3.2 类**
    1) self参数是方法和函数的区别; 方法将它们的第一个参数绑定到所属的实例上,
       因此可以不提供该参数; 因此可以将特性绑定到一个普通函数上, 这样就可以
       不会有特殊self参数了;
          class Class:
            def method(self):
              print 'have self'
          def function():
            print 'nothing'
          ins = Class()
          ins.method()
          ins.method = function
          ins.method()
    2) 为了让方法或特性变为私有, 只要在它的名字前面加上双下划线即可;
       所有以双下划线开始的名字都会被"翻译"成前面加单下划线和类名字的形式;
    3) 类中没有加self限定的变量, 类似于静态成员; 但是如果以某实例重新绑定了
       该变量的值, 那么只会影响该实例;
    4) 面向对象的思考:
        * 属于一类的对象放在一起; 
        * 不要让对象过于亲密;
        * 小心继承, 尤其是多继承;
        * 简单就好, 让你的方法小巧;
    5) 函数:
        * callable(object)                  确定对象是否可被调用
        * getattr(object, name[, default])  确定特性的值, 可选择提供默认值
        * hasattr(object, name)             确定对象是否有给定的特性
        * isinstance(object, class)         确定对象是否时类的实例
        * issubclass(A, B)                  确定A是否为B的子类
        * random.choice(sequence)           从非空序列中随机选择元素
        * setattr(object,name,value)        设定对象的给定特性为value
        * type(object)                      返回对应的类型



## **4. 异常**
    1) 内建异常
        * Exception           所有异常的基类
        * AttributeError      特性引用或赋值失败时引发
        * IOError             试图打开不存在的文件时引发
        * IndexError          时使用序列中不存在的索引时引发
        * KeyError            在使用映射中不存在的键时引发
        * NameError           在找不到名字时引发
        * SyntaxError         在代码为错误形式时引发
        * TypeError           在内建操作或函数应用于错误类型的对象时引发
        * ValueError
          在内建操作或函数应用于正确类型的对象, 但该对象使用不合适的值时引发
        * ZeroDivisionError   在除法或取模操作的第二个参数为0时引发
    2) 如果捕捉到了异常, 但又想重新引发它, 可以调用不带参数的raise来实现;
    3) 除了except语句还可以使用else子句; 如果try块中没有引发异常, else子句就
       会被执行;
    4) warings.filterwarnings(action, ...)  用于过滤警告




## **5. 魔法方法, 属性和迭代器**
    Python里名字前面和后面都有两个下划线的方法称为魔法方法; 为了确保类是新型
    的, 应该把赋值语句__metaclass__ = type放在模块的最开始, 或子类化内建类
    object;
    1) 构造方法__init__, 会被对象自动调用; 在重写中一个类能通过实现方法来重
       写超类中定义的方法和属性; 新方法要调用重写版本的方法, 可以从超类直接
       调用未绑定的版本或使用super函数;
    2) 新函数
        * iter                从一个可迭代对象得到迭代器
        * property(fget,fset,fdel,doc)
          返回一个属性, 所有的参数都是可选的
        * super(class,obj)    返回一个类的超类的绑定实例



## **6. 模块**
> ### **6.1 模块**
    1) sys.path.append()添加模块需要是模块的全路径, 如希望传递一个相对路径而
       由Pytho自动补全路径, 可以使用sys.path.expanduser()
    2) 导入模块主要用于定义, 而只需要定义这些东西一次, 所以导入模块多次和导
       入一次效果是一样的;
    3) 让sys.path一开始就包含正确的目录:
        * 将模块放在合适的位置
          查看sys.path; (site-packages是最佳的模块目录); 一般放site-packages
        * 告诉解释器去那里查找需要的模块
          不适合将'模块放在正确的位置的原因'
            a. 不希望将自己的模块填满Python解释器的目录;
            b. 没有在Python解释器目录中存储文件的权限;
            c. 想将模块放在其他地方;
          设置PYTHONPATH环境变量(如):
            export PYTHONPATH=$PYTHONPATH:~/Python 

> ### **6.2 包**
    1) 为了组织好模块, 可以将它们分组成包; 包是另外一类模块, 能包含其他模块,
       包就是模块所在的目录, 为了让Python将其作为包对待, 包中必须包含一个命
       名为__init__.py的特殊文件;

> ### **6.3 探究模块**
    1) 探究模块中有些什么
        * 使用dir: 查看模块中包含的内容
        * __all__: 定义了模块的公共接口,告诉解释器从模块导入所有名字代表什么
                   在模块中设置__all__可以过滤掉一些其他程序不需要或不想要的
                   变量, 函数和类;
    2) 使用help函数获取帮助, 使用help比module.__doc__能获取更多的帮助信息;
    3) 使用模块的__file__属性可以得知模块的源代码在什么地方;

> ### **6.4 标准库**
    1) sys模块中的一些重要函数和变量
        * argv          命令行参数, 包括脚本名称
        * exit([argv])  退出当前程序, 可选参数为给定的返回值或错误信息
        * modules       映射模块名字到载入模块的字典
        * path          查找模块所在目录的目录名列表
        * platform      类似sunos5或win32的平台标识符
        * stdin         标准输入流——一个类文件对象
        * stdout        标准输出流——一个类文件对象
        * stderr        标准错误流——一个类文件对象
       Demo: ./src/reverse_args.py
    2) os模块中一些重要函数和变量
        * environ           对环境变量进行映射
        * system(command)   在子shell中执行操作系统命令
          在windows中如果路径有空格,可以使用一个更好的WIN特有函数startfile;
          对于打开浏览器更好的方法是使用webbrowser模块;
              webbrowser.open('http://www.baidu.com')
        * sep               路径中的分隔符
        * pathsep           分隔路径的分隔符
        * linesep           行分隔符('\n', '\r', '\r\n')
        * urandom(n)        返回n字节的加密随机数据
    3) fileinput模块中重要的函数
        * input([files[, inplace[, backup]]])
          便于遍历多个输入流中的行
        * filename()        返回当前文件的名称
        * lineno()          返回当前(累计)的行数
        * filelineno()      返回当前文件的行数
        * isfirstline()     检查当前行是否是文件的第一行
        * isstdin()         检查最后一行是否来自sys.stdin
        * nextfile()        关闭当前文件, 移到下一个文件
        * close()           关闭序列
        DEMO: ./src/number_lines.py
