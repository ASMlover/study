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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../lib/queue.h"
#include "dancing_partner.h"


#define DANCER_FMALE      (0)
#define DANCER_MALE       (1)
#define DANCER_NUMBER     (100)
#define DANCING_MUSICS    (5)

typedef struct {
  int id;
  int sex;
} dancer_t;


static void 
build_dancers(dancer_t* dancers, int dancer_num) 
{
  int i;
  if (NULL == dancers || dancer_num <= 0) {
    fprintf(stderr, "argument invalid ...");
    exit(0);
  }

  srand(time(0));
  for (i = 0; i < dancer_num; ++i) {
    dancers[i].id  = i;
    dancers[i].sex = rand() % 2;
  }
}


void 
dancing_partner(void)
{
  dancer_t dancers[DANCER_NUMBER];
  queue_t queue_male = queue_create();
  queue_t queue_famle = queue_create();
  int i, j, idx;
  int dancers_per_music = DANCER_NUMBER / DANCING_MUSICS;
  dancer_t* male;
  dancer_t* famle;

  build_dancers(dancers, DANCER_NUMBER);
  for (i = 0; i < DANCING_MUSICS; ++i) {
    for (j = 0; j < dancers_per_music; ++j) {
      idx = i * dancers_per_music + j;
      if (DANCER_MALE == dancers[idx].sex)
        queue_enqueue(queue_male, &dancers[idx]);
      else 
        queue_enqueue(queue_famle, &dancers[idx]);
    }

    while (!queue_empty(queue_male) && !queue_empty(queue_famle)) {
      male = (dancer_t*)queue_dequeue(queue_male);
      famle = (dancer_t*)queue_dequeue(queue_famle);

      fprintf(stdout, "music [%d], dancers {male=>%d, famle=>%d}\n", 
        i, male->id, famle->id);
    }
  }

  fprintf(stdout, "\n");
  while (!queue_empty(queue_male)) {
    male = (dancer_t*)queue_dequeue(queue_male);
    fprintf(stdout, "no dancing dancer : {id=>%d, sex=>male}\n", male->id);
  }
  while (!queue_empty(queue_famle)) {
    famle = (dancer_t*)queue_dequeue(queue_famle);
    fprintf(stdout, "no dancing dancer : {id=>%d, sex=>famle}\n", famle->id);
  }

  queue_delete(&queue_famle);
  queue_delete(&queue_male);
}
