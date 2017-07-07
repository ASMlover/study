# **README for allocator2** #
***


## **What's allocator2?** ##
 * It's a pool of memory allocator (recorded memory index in each block).
 * It's a open source and free library based on FreeBSD license.


## **Building allocator2** ##
 * **windows**

        step1: Setting environment for using MSVC
        step2:
          \> cd allocator2
          \> nmake -f makefile.win 

 * **linux**

          $ cd allocator2
          $ make 



## **How to use allocator2?** ##
 * **step**

        * step1: call allocator_init() at beginning of your program
        * step2: 
          use al_malloc() to allocate memory from the pool of allocator;
          use al_free() to recycle memory and put it back to allocator.
        * step3: call allocator_destroy() at ending of your program.

 * **demo**

        see ./allocator_test.c
