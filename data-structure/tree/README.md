# **README for Tree**
***

## **二叉搜索树**
设x是二叉搜索树中的一个节点，如果y是x的左子树中的一个节点，那么y.key <= x.key；如果y是x右子树中的一个节点，那么y.key >= x.key；

**中序遍历**如下：如果x是一棵有n各节点子树的根，那么INORDER-TREE-WALK(x)需要O(n)时间
```
INORDER-TREE-WALK(x)
  if x != NIL
    INORDER-TREE-WALK(x.left)
    print x.key
    INORDER-TREE-WALK(x.right)
```

**查找**
```
TREE-SEARCH(x, k)
  if x == NIL or k == x.key
    return x
  if k < x.key
    return TREE-SEARCH(x.left, k)
  else return TREE-SEARCH(x.right, k)

ITERATIVE-TREE-SEARCH(x, k)
  while x != NIL and k != x.key
    if k < x.key
      x = x.left
    else x = x.right
  return x
```

**最大关键字元素和最小关键字元素**
```
TREE-MINIMUM(x)
  while x.left != NIL
    x = x.left
  return x

TREE-MAXIMUM(x)
  while x.right != NIL
    x = x.right
  return x
```

**后继与前驱**，后继分2种情况，如果x的右子树非空，那么x的后继是x右子树的最左节点；如果x的右子树非空并且有一个后继y，那么y就是x的有左孩子的最底层祖先，其是x的一个祖先；前驱与后继相对称
```
TREE-SUCCESSOR(x)
  if x.right != NIL
    return TREE-SUCCESSOR(x.right)

  y = x.p
  while y != NIL and x == y.right
    x = y
    y = y.p
  return y

TREE-PREDECESSOR(x)
  if x.left != NIL
    return TREE-PREDECESSOR(x.left)

  y = x.p
  while y != NIL and x == y.left
    x = y
    y = y.p
  return y
```
