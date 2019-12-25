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

## **红黑树**
**红黑树**是一棵满足以下性质的二叉搜索树：
 * 每个节点或是红色，或是黑色
 * 根节点是黑色
 * 每个叶子节点（NIL）是黑色
 * 如果一个节点是红色，它的两个子节点都是黑色
 * 对每个节点，从该节点到其所有后代叶子节点的简单路径上，包含相同数目的黑色节点

**旋转**，当在x上做左旋时，假设其右孩子是y而不是NIL；x可以为其右孩子不是NIL节点的树内任意节点；左旋以x到y的链为之轴进行，使y称为该子树新的根节点，x成y的左孩子，y的左孩子成为x的右孩子；
```
        |                         |
        y    <---LEFT-ROTATE---   x
       /     ---RIGHT-ROTATE-->  / \
      x                         a   y
     / \                           /
    a   b                         b

LEFT-ROTATE(T, x)
  y = x.right
  x.right = y.left
  if y.left != NIL
    y.left.p = x
  y.p = x.p
  if x.p == NIL
    T.root = y
  elseif x == x.p.left
    x.p.left = y
  else x.p.right = y
  y.left = x
  x.p = y

RIGHT-ROTATE(T, x)
  x = y.left
  y.left = x.right
  if x.right != NIL
    x.right.p = y
  x.p = y.p
  if y.p == NIL
    T.root = x
  elseif y.p.left = y
    y.p.left = x
  else y.p.right = x
  x.right = y
  y.p = x
```

**插入**，需要花费O(lgn)的时间
```
RB-INSERT(T, z)
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
  z.left = NIL
  z.right = NIL
  z.color = RED
  RB-INSERT-FIXUP(T, z)

RB-INSERT-FIXUP(T, z)
  while z.p.color == RED
    if z.p == z.p.p.left
      y = z.p.p.right
      if y.color == RED
        z.p.color = BLACK               -> case 1
        y.color = BLACK                 -> case 1
        z.p.p.color = RED               -> case 1
        z = z.p.p                       -> case 1
      else if z == z.p.right
          z = z.p                       -> case 2
          LEFT-ROTATE(T, z)             -> case 2
        z.p.color = BLACK               -> case 3
        z.p.p.color = RED               -> case 3
        RIGHT-ROTATE(T, z.p.p)          -> case 3
    else (same as the then clause with "right" and "left" exchanged)
  T.root.color = BLACK
```
INSERT-FIXUP的几种情况（需要保证z是新插入的节点）：
 * case1：z的叔叔节点y是红色的：
   - 将z.p和y都着为黑色，来解决z和z.p都是红色的问题
   - 将z.p.p着为红色来保持性质5
   - 将z.p.p作为新节点z来重复while循环
 * case2：z的叔叔节点y是黑色的且z是一个右孩子
 * case3：z的叔叔节点y是黑色的且z是一个左孩子
   - case2和case3，z的叔节点y是黑色，通过z是z.p的右孩子还是左孩子来区别这两种情况
   - case2中z是它父节点的右孩子，立即实用一个左旋将此情形转换为case3
   - case3中改变某些节点的颜色再做一次右旋

**删除**，需要花费O(lgn)的时间
 * 始终维持y是从树中删除的节点或移至树内的节点，当z子节点少于2个时，将y指向z，并因此要移除；当z有2个子节点时，将y指向z的后继，y将移动到树中z的位置
 * 由于y可能变色，y-original-color存储变化前y的颜色，当z有2个子节点时，则y!=z且节点y移至节点z的原始位置，给y赋予和z一样的颜色；在RB-DELETE结束的时候检测y-original-color的颜色，如果它是黑色的，那需要移动或删除y会引起红黑树性质的破坏
 * 保存节点x的踪迹，使它移至节点y的原始位置上
 * 因为x移到y的原始位置，x.p总是被设置指向树中y父节点的原始位置，除非z是y的原始父节点；当y的原父节点是z时，因为要删除该节点，则不能让x.p指向y的原始父节点；由于y将在树中向上移动占据z的位置，则将x.p设置为y，使得x.p指向y父节点的原始位置
 * 最后如果y是黑色，可能引入一个或多个红黑树性质的破坏，所以需要进行修正
```
// 删除过程基于TREE-DELETE过程，需要一个特别的TRANSPLANT
RB-TRANSPLANT(T, u, v)
  if u.p == NIL
    T.root = v
  elseif u == u.p.left
    u.p.left = v
  else u.p.right = v
  v.p = u.p

RB-DELETE(T, z)
  y = z
  y-original-color = y.color
  if z.left == NIL
    x = z.right
    RB-TRANSPLANT(T, z, z.right)
  elseif z.right == NIL
    x = z.left
    RB-TRANSPLANT(T, z, z.left)
  else y = TREE-MINIMUM(z.right)
    y-original-color = y.color
    x = y.right
    if y.p == z
      x.p = y
    else RB-TRANSPLANT(T, y, y.right)
      y.right = z.right
      y.right.p = y
    RB-TRANSPLANT(T, z, y)
    y.left = z.left
    y.left.p = y
    y.color = z.color

  if y-original-color == BLACK
    RB-DELETE-FIXUP(T, x)
```
