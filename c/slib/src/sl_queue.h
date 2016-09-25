#ifndef __SL_QUEUE_HEADER_H__
#define __SL_QUEUE_HEADER_H__

typedef struct sl_queue_t sl_queue_t;


/*
 * @attention:
 *    All interfaces of queue module, 
 *    you must ensure the validity of the 
 *    incoming parameters.
 */
extern sl_queue_t* sl_queue_create(void);
extern void sl_queue_release(sl_queue_t* queue);

extern size_t sl_queue_size(sl_queue_t* queue);
extern void sl_queue_push(sl_queue_t* queue, void* value);
extern void* sl_queue_pop(sl_queue_t* queue);

#endif  /* __SL_QUEUE_HEADER_H__ */
