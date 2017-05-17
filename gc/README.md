# **README for gc**
***

## **Summary**
  * Some algorithms about **GC**.
  * **GC** demos name based **Njord**.

## **Spin-Lock**
1. AtomicExchange spin lock
```shell
ExchangeLock(x):
  while AtomicExchange(x, 1) == 1
    /* do nothing */

ExchangeUnlock(x):
  *x <- 0

AtomicExchange(x, v):
  atomic
    old <- *x
    *x <- v
  return old
```

2. Test and test and set AtomicExchange spin lock
```sell
TestAndTestAndSetExchangeLock(x):
  while TestAndExchange(x) == 1
    /* do nothing */

TestAndTestAndSetExchangeUnlock(x):
  *x <- 0

TestAndExchange(x):
  while *x == 1
    /* do nothing */
  return AtomicExchange(x, 1)
```
