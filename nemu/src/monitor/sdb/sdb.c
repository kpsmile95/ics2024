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
#include <cpu/cpu.h>
#include <isa.h>
#include <memory/vaddr.h>
#include <readline/history.h>
#include <readline/readline.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void sdb_watchpoint_display();
void new_watchpoint(char *addr);
bool del_watchpoint(int wp_no);

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets() {
    static char *line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read) {
        add_history(line_read);
    }

    return line_read;
}

static int cmd_c(char *args) {
    cpu_exec(-1);
    return 0;
}

static int cmd_q(char *args) {
    nemu_state.state = NEMU_QUIT;
    return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
    char *step = strtok(args, " ");
    char *moreArg = strtok(NULL, " ");
    if (moreArg != NULL) {
        printf("Does not match the input format: si [N] .    error:1\n");
    } else if (step == NULL) {
        cpu_exec(1);
    } else {
        int num;
        int result = sscanf(step, "%d", &num);
        if (result == 1) {
            cpu_exec(num);
        } else {
            printf("Does not match the input format: si [N] .    error:1\n");
        }
    }
    return 0;
}

static int cmd_info(char *args) {
    char *arg0 = strtok(args, " ");
    char *moreArg = strtok(NULL, " ");
    if (moreArg != NULL || arg0 == NULL) {
        printf("Does not match the input format: info SUBCMD.    error:1\n");
    } else {
        if (*arg0 == 'r') {
            isa_reg_display();
        } else if (*arg0 == 'w') {
            sdb_watchpoint_display();
        } else {
            printf("Unknow command!\n");
        }
    }
    return 0;
}

static int cmd_x(char *args) {
    char *nums = strtok(args, " ");
    char *addr_ = strtok(NULL, " ");
    char *moreArg = strtok(NULL, " ");
    if (moreArg != NULL || nums == NULL || addr_ == NULL) {
        printf("Does not match the input format: x N EXPR .    error:1\n");
    } else {
        int i;
        vaddr_t addr;
        sscanf(nums, "%d", &i);
        sscanf(addr_, "%x", &addr);
        for (int j = 0; j < i; j++) {
            printf("address: %x  memory: %08x\n", addr, vaddr_read(addr, 4));
            addr += 4;
        }
    }
    return 0;
}

static int cmd_w(char *args) {
    char *addr = strtok(args, " ");
    char *moreArg = strtok(NULL, " ");
    if (moreArg != NULL || addr == NULL) {
        printf("Does not match the input format: w EXPR .    error:1\n");
    } else {
        // todo : valid addr
        new_watchpoint(addr);
    }
    return 0;
}

static int cmd_p(char *args) {
    bool success;
    uint32_t v = expr(args, &success);
    if (success)
        printf("%s = \e[1;36m%u\e[0m\n", args, v);
    return 0;
}

static int cmd_d(char *args) {
    char *addr = strtok(args, " ");
    char *moreArg = strtok(NULL, " ");
    if (moreArg != NULL || addr == NULL) {
        printf("Does not match the input format: w EXPR .    error:1\n");
    } else {
        // todo : valid addr
        int NO;
        sscanf(args, "%d", &NO);
        del_watchpoint(NO);
    }
    return 0;
}

static struct {
    const char *name;
    const char *description;
    int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "Single-Step execution", cmd_si},
    {"info", "Print program status", cmd_info},
    {"x", "Scan memory", cmd_x},
    {"w", "Set watchpoint", cmd_w},
    {"p", "Evaluate the expression EXPR", cmd_p},
    {"d", "Delete watchpoint", cmd_d},

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
    /* extract the first argument */
    char *arg = strtok(NULL, " ");
    int i;

    if (arg == NULL) {
        /* no argument given */
        for (i = 0; i < NR_CMD; i++) {
            printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    } else {
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(arg, cmd_table[i].name) == 0) {
                printf("%s - %s\n", cmd_table[i].name,
                       cmd_table[i].description);
                return 0;
            }
        }
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
    if (is_batch_mode) {
        cmd_c(NULL);
        return;
    }

    for (char *str; (str = rl_gets()) != NULL;) {
        char *str_end = str + strlen(str);

        /* extract the first token as the command */
        char *cmd = strtok(str, " ");
        if (cmd == NULL) {
            continue;
        }

        /* treat the remaining string as the arguments,
         * which may need further parsing
         */
        char *args = cmd + strlen(cmd) + 1;
        if (args >= str_end) {
            args = NULL;
        }

#ifdef CONFIG_DEVICE
        extern void sdl_clear_event_queue();
        sdl_clear_event_queue();
#endif

        int i;
        for (i = 0; i < NR_CMD; i++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
                if (cmd_table[i].handler(args) < 0) {
                    return;
                }
                break;
            }
        }

        if (i == NR_CMD) {
            printf("Unknown command '%s'\n", cmd);
        }
    }
}

void init_sdb() {
    /* Compile the regular expressions. */
    init_regex();

    /* Initialize the watchpoint pool. */
    init_wp_pool();
}
