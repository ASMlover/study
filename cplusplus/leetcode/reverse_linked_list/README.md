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
