```
[1]->[2]->[3]->[4]->[5]->NIL
 ^
 |
head
```

###### [迭代解法]
 * 设置指针[prev]为NIL，指针[next]为NIL
 * 设置循环，直到[curr]迭代到尾巴上，返回[prev]
 * 设置[next]为[curr]的next，[curr]的next为[prev]
 * 设置[prev]指向[curr]，[curr]指向[next]
```
   c
  [1] -> [2] -> [3] -> [4] -> [5] -> [ ]

   p      n
  [ ]    [ ]

  n = c->next         n
 [1]->next        => [2]         <-----------.
                                             |
  c->next = p         c                      |
 [1]->next->p     => [1] -> [ ]              |
                                             |
  p = c               p                      |
 [ ]->c           => [1] -> [ ]              |
                                             |
  c = n               c                      |
 [1]->n           => [2]         ------------'
```

###### [递归解法]
 * [curr]为NIL或[next]为NIL，则直接返回
 * [prev]设置为[next]的递归调用返回结果
 * [next]的next指向[curr]
 * [curr]的next设置为NIL
```
   c
  [1] -> [2] -> [3] -> [4] -> [5] -> [ ]

 c == NIL || c->next == NIL => return c   <---.
                                              |
  n = c->next         n                       |
 [1]->next        => [2]                      |
                                              |
  p = ----------------------------------------'

  n->next = c         n
 [2]->next->[1]   => [2] -> [1]

  c->next = NIL       n
 [1]->next->[ ]   => [2] -> [1] -> [ ]
```
