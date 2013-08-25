# **README for allocator** #
***


## **What's allocator?** ##
 * It's a pool of memory allocator implemented by C.
 * It's a open source and free library.


## **Building allocator** ##
 * **linux**

        $ cd allocator
        $ make 

 * **windows**

        \> cd allocator
        \> nmake -f makefile.win 



## **Use allocator** ##
 * **step**

        * step1: call allocator_init() at the beginning
        * step2: call allocator_destroy() at the ending
        * step3: call al_malloc()/al_free where you want to use

 * **Demo**

        see ./allocator_test.c
