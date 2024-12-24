/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  struct watchpoint *pre;
  uint32_t old_value;
  uint32_t new_value;
  char *wp_var;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
static uint8_t wp_size;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].pre = (i == 0 ? NULL : &wp_pool[i - 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void sdb_watchpoint_display() {
  WP *p = head;
  if (p == NULL) {
    printf("No watchpoint!\n");
    return;
  }
  while (p != NULL) {
    printf("%d %s\n", p->NO, p->wp_var);
    p = p->next;
  }
}

WP *new_wp() {
  if (free_ == NULL) {
    /* code */
    printf("no more free wp\n");
    assert(0);
  }
  WP *wp = free_;
  free_ = free_->next;

  if (head == NULL) {
    head = wp;
  } else {
    WP *last = head;
    while (last->next) {
      last = last->next;
    }
    last->next = head;
  }
  wp->next = NULL;
  return wp;
}

void new_watchpoint(char *addr) {
  WP *wp = new_wp();
  wp->wp_var = strdup(addr);
  bool success = true;
  wp->old_value = expr(addr, &success);
  wp->new_value = wp->old_value;
  wp->NO = wp_size++;
  printf("New watchpoint %d: %s\n", wp->NO, wp->wp_var);
}

void del_watchpoint(int wp_no) {
  WP *tmp = head;
  while (tmp) {
    if (tmp->NO == wp_no) {
      break;
    }
    tmp = tmp->next;
  }
  if (tmp == NULL) {
    printf("No %d watchpoint!", wp_no);
  } else {
    WP *next = tmp->next;
    WP *pre = tmp->pre;
    if (next != NULL) {
      next->pre = pre;
    }
    if (pre != NULL) {
      pre->next = next;
    }
    tmp->NO = 0;
    tmp->next = free_;
    free_->pre = tmp;
    free_ = tmp;
  }
}