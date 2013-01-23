#ifndef __BTREE_HEADER_H__
#define __BTREE_HEADER_H__ 

typedef struct btree_s* btree_t;
typedef void* element_t;


extern btree_t btree_create(void);
extern void btree_delete(btree_t* bt);

extern int btree_empty(btree_t bt);
extern int btree_size(btree_t bt);
extern void btree_clear(btree_t bt);
extern void btree_insert(btree_t bt, int key, element_t val);
extern element_t btree_remove(btree_t bt, int key);
extern element_t btree_get(btree_t bt, int key);
extern void btree_preorder(btree_t bt, void (*visit)(int, element_t));
extern void btree_inorder(btree_t bt, void (*visit)(int, element_t));
extern void btree_postorder(btree_t bt, void (*visit)(int, element_t));

#endif  /* __BTREE_HEADER_H__ */
