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

**插入**，z作为输入(z.key=v, z.left=NIL, z.right=NIL)
```
TREE-INSERT(T, z)
  y = NIL
  x = T.root
  while x != NIL
    y = x
    if z.key < x.key
      x = x.left
    else x = x.right

  z.p = y
  if y == NIL
    T.root = z
  elseif z.key < y.key
    y.left = z
  else y.right = z
```

**删除**，假设要删除的节点为z
 * z只有右孩子，直接用右孩子替换z；
 * z只有左孩子，直接用左孩子替换z；
 * z既有一个左孩子又有一个右孩子，需要找到z的后继y（位于z的右子树且没有左孩子），需要将y移出原来的位置进行拼接，并替换z；
   - y是z的右孩子，用y替换z，并仅留下y的右孩子；
   - y是z的右子树中但并非z的右孩子，先用y右孩子替换y，再用y替换z；
```
// TRANSPLANT -> 用另一棵子树替换一棵子树并称为其双亲的孩子节点(u->old, v->new)
TRANSPLANT(T, u, v)
  if u.p == NIL
    T.root = v
  elseif u == u.p.left
    u.p.left = v
  else u.p.right = v

  if v != NIL
    v.p = u.p

TREE-DELETE(T, z)
  if z.left == NIL
    TRANSPLANT(T, z, z.right)
  elseif z.right == NIL
    TRANSPLANT(T, z, z.left)
  else y = TREE-MINIMUM(z.right)
    if y.p != z
      TRANSPLANT(T, y, y.right)
      y.right = z.right
      y.right.p = y
    TRANSPLANT(T, z, y)
    y.left = z.left
    y.left.p = y
```
