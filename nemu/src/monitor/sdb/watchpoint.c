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

enum { WATCHPOINT, BREAKPOINT };

typedef struct watchpoint {
    uint8_t ori_byte : 8;
    bool enable : 1;
    bool in_use : 1;
    int NO : 22;

    union {
        vaddr_t addr;
        struct {
            char *expr;
            uint32_t old_val;
            uint32_t new_val;
        };
    };
    int type;
    struct watchpoint *next;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].in_use = false;
        wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
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
        printf("%d %s\n", p->NO, p->expr);
        p = p->next;
    }
}

WP *new_wp() {
    assert(free_ != NULL);
    WP *wp = free_;
    free_ = free_->next;
    assert(wp->in_use == false);
    wp->in_use = true;
    return wp;
}

static void free_wp(WP *p) {
    assert(p >= wp_pool && p < wp_pool + NR_WP);
    assert(p->in_use == true);
    if (p->type == WATCHPOINT)
        free(p->expr);
    // else
    // vaddr_write(p->addr, SREG_CS, 1, p->ori_byte);
    p->in_use = false;
    p->next = free_;
    free_ = p;
}

int new_watchpoint(char *addr) {
    uint32_t val;
    bool success;
    val = expr(addr, &success);
    if (!success)
        return -1;

    WP *p = new_wp();
    p->type = WATCHPOINT;
    p->expr = strdup(addr);
    p->old_val = val;
    p->enable = true;

    p->next = head;
    head = p;

    return p->NO;
}

bool del_watchpoint(int wp_no) {
    WP *p, *prev = NULL;
    for (p = head; p != NULL; prev = p, p = p->next) {
        if (p->NO == wp_no) {
            break;
        }
    }

    if (p == NULL) {
        return false;
    }
    if (prev == NULL) {
        head = p->next;
    } else {
        prev->next = p->next;
    }

    free_wp(p);
    return true;
}