# **README for gc**
***

## **Summary**
  * Some algorithms about **GC**.
  * **GC** demos name based **Njord**.

## **SpinLocks**
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
```shell
TestAndTestAndSetExchangeLock(x):
  while TestAndExchange(x) = 1
    /* do nothing */

TestAndTestAndSetExchangeUnlock(x):
  *x <- 0

TestAndExchange(x):
  while *x = 1
    /* do nothing */
  return AtomicExchange(x, 1)
```
3. TestAndSet primitive implement spin lock
```shell
TestAndSetLock(x):
  while TestAndSet(x) = 1
    /* do nothing */

TestAndSetUnlock(x):
  *x <- 0

TestAndSet(x):
  atomic
    old <- *x
    if old = 0
      *x = 1
      return 0
    return 1

TestAndTestAndSetLock(x):
  while TestAndTestAndSet(x) = 1
    /* do nothing */

TestAndTestAndSetUnlock(x):
  TestAndSetUnlock(x)

TestAndTestAndSet(x):
  while *x = 1
    /* do nothing */
  return TestAndSet(x)
```
