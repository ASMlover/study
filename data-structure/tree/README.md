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
