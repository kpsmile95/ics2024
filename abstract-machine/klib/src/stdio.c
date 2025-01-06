#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// 辅助函数：将整数转换为字符串
static void itoa(int value, char *str) {
    char *p = str;
    char *p1, *p2;
    int tmp_value;

    if (value < 0) {
        value = -value;
        *p++ = '-';
    }

    p1 = p;
    do {
        tmp_value = value;
        value /= 10;
        *p++ = '0' + (tmp_value - value * 10);
    } while (value);

    *p = '\0';

    // 反转字符串
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

int printf(const char *format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    int result = vsprintf(buffer, format, args);
    va_end(args);
    // 输出到标准输出
    for (int i = 0; buffer[i] != '\0'; i++) {
        putch(buffer[i]);
    }
    return result;
}

int vsprintf(char *str, const char *format, va_list args) {
    int i = 0, j = 0;
    char buffer[100];
    char *str_arg;
    int int_arg;

    while (format[i] != '\0') {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
            case 'd':
                int_arg = va_arg(args, int);
                itoa(int_arg, buffer);
                for (int k = 0; buffer[k] != '\0'; k++) {
                    str[j++] = buffer[k];
                }
                break;
            case 's':
                str_arg = va_arg(args, char *);
                while (*str_arg) {
                    str[j++] = *str_arg++;
                }
                break;
            // 可以根据需要添加更多格式化选项
            default:
                str[j++] = format[i];
                break;
            }
        } else {
            str[j++] = format[i];
        }
        i++;
    }
    str[j] = '\0';
    return j;
}

int sprintf(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsprintf(str, format, args);
    va_end(args);
    return result;
}

int vsnprintf(char *str, size_t size, const char *format, va_list args) {
    int i = 0, j = 0;
    char buffer[100];
    char *str_arg;
    int int_arg;

    while (format[i] != '\0' && j < size - 1) {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
            case 'd':
                int_arg = va_arg(args, int);
                itoa(int_arg, buffer);
                for (int k = 0; buffer[k] != '\0' && j < size - 1; k++) {
                    str[j++] = buffer[k];
                }
                break;
            case 's':
                str_arg = va_arg(args, char *);
                while (*str_arg && j < size - 1) {
                    str[j++] = *str_arg++;
                }
                break;
            // 可以根据需要添加更多格式化选项
            default:
                if (j < size - 1) {
                    str[j++] = format[i];
                }
                break;
            }
        } else {
            if (j < size - 1) {
                str[j++] = format[i];
            }
        }
        i++;
    }
    str[j] = '\0';
    return j;
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(str, size, format, args);
    va_end(args);
    return result;
}

#endif
