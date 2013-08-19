/*
 * Copyright (c) 2013 ASMlover. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list ofconditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materialsprovided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __PROTECT_HEADER_H__
#define __PROTECT_HEADER_H__

/* 
 * descriptor about store data segment 
 * descriptor about system segment
 */
typedef struct descriptor_s {
  uint16_t  limit_low;        /* limit */
  uint16_t  base_low;         /* base  */
  uint8_t   base_mid;         /* base  */
  uint8_t   attr1;            /* P(1) DPL(2) DT(1) TYPE(4) */
  uint8_t   limit_high_attr2; /* G(1) D(1) O(1) AVL(1) LimitHigh(4) */
  uint8_t   base_high;        /* base  */
} descriptor_t;

/* gate descriptor */
typedef struct gate_s {
  uint16_t  offset_low;       /* offset low */
  uint16_t  selector;         /* selector */
  uint8_t   dcount;
  uint8_t   addr;             /* P(1) DPL(2) DT(1) TYPE(4) */
  uint16_t  offset_high;      /* offset high */
} gate_t;


/*
 * GDT 
 * index of descriptor
 */
#define INDEX_DUMMY         (0)
#define INDEX_FLAG_C        (1)
#define INDEX_FLAG_RW       (2)
#define INDEX_VIDEO         (3)

/* selector */
#define SELECTOR_DUMMY      (0)
#define SELECTOR_FLAT_C     (0x08)
#define SELECTOR_FLAT_RW    (0x10)
#define SELECTOR_VIDEO      (0x18 + 3)
#define SELECTOR_KERNEL_CS  (SELECTOR_FLAT_C)
#define SELECTOR_KERNEL_DS  (SELECTOR_FLAT_RW)

/* summary about type of descriptor */
#define DA_32               (0x4000)  /* 32bits segment */
#define DA_LIMIT_4K         (0x8000)  /* limited of segment 4K */
#define DA_DPL0             (0x00)    /* DPL = 0 */
#define DA_DPL1             (0x20)    /* DPL = 1 */
#define DA_DPL2             (0x40)    /* DPL = 2 */
#define DA_DPL3             (0x60)    /* DPL = 3 */
/* memory segment type descriptor */
#define DA_DR               (0x90)    /* read only */
#define DA_DRW              (0x92)    /* read/write */
#define DA_DRWA             (0x93)    /* accessed read/write */
#define DA_C                (0x98)    /* execute only */
#define DA_CR               (0x9A)    /* execute/read */
#define DA_CCO              (0x9C)    /* execute once */
#define DA_CCOR             (0x9E)    /* execute once/read */
/* system segment type descriptor */
#define DA_LDT              (0x82)    /* local descriptor */
#define DA_TASKGATE         (0x85)    /* task gate */
#define DA_386TSS           (0x89)    /* 386 task status segment */
#define DA_386CGATE         (0x8C)    /* 386 call gate */
#define DA_386IGATE         (0x8E)    /* 386 interrupt gate */
#define DA_386TGATE         (0x8F)    /* 386 trap gate */


/* interrupt vector */
#define INT_VECTOR_DIVIDE       (0x0)
#define INT_VECTOR_DEBUG        (0x1)
#define INT_VECTOR_NMI          (0x2)
#define INT_VECTOR_BREAKPOINT   (0x3)
#define INT_VECTOR_OVERFLOW     (0x4)
#define INT_VECTOR_BOUNDS       (0x5)
#define INT_VECTOR_INVAL_OP     (0x6)
#define INT_VECTOR_COPROC_NOT   (0x7)
#define INT_VECTOR_DOUBLE_FAULT (0x8)
#define INT_VECTOR_COPROC_SEG   (0x9)
#define INT_VECTOR_INVAL_TSS    (0xA)
#define INT_VECTOR_SEG_NOT      (0xB)
#define INT_VECTOR_STACK_FAULT  (0xC)
#define INT_VECTOR_PROTECTION   (0xD)
#define INT_VECTOR_PAGE_FAULT   (0xE)
#define INT_VECTOR_COPROC_ERR   (0x10)

#define INT_VECTOR_IRQ0         (0x20)
#define INT_VECTOR_IRQ8         (0x28)


#endif  /* __PROTECT_HEADER_H__ */
