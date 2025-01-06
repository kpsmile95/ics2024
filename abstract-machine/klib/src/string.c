#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *str) {
    const char *char_ptr;
    const uint32_t *word_ptr;
    register uint32_t word, magic_bits;
    for (char_ptr = str; ((uint32_t)char_ptr & (sizeof(uint32_t) - 1)) != 0;
         ++char_ptr) {
        if (*char_ptr == '\0')
            return char_ptr - str;
    }

    word_ptr = (uint32_t *)char_ptr;

    magic_bits = 0x7efefeffL;

    while (1) {
        word = *word_ptr++;
        if ((((word + magic_bits) ^ ~word) & ~magic_bits) != 0) {
            const char *cp = (const char *)(word_ptr - 1);
            if (cp[0] == 0)
                return cp - str;
            if (cp[1] == 0)
                return cp - str + 1;
            if (cp[2] == 0)
                return cp - str + 2;
            if (cp[3] == 0)
                return cp - str + 3;
        }
    }
}

char *strcpy(char *dst, const char *src) {
    char *original_dst = dst;           // 保存原始的 dst 以便返回
    memmove(dst, src, strlen(src) + 1); // 复制字符串并包含结束符
    return original_dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    char *original_dst = dst; // 保存原始的 dst 以便返回
    memmove(dst, src, n);     // 复制字符串并包含结束符
    return original_dst;
}

char *strcat(char *dst, const char *src) {
    assert(dst != NULL && src != NULL); // 检查指针的有效性

    char *tmp = dst; // 记下dest的初始地址防止找不到

    while (*dst)
        dst++; // 找到dest的末尾
    while ((*dst++ = *src++) != '\0')
        ; // 将src附加到dest后面

    return tmp;
}

int strcmp(const char *src, const char *dst) {

    register const unsigned char *s1 = (const unsigned char *)src;
    register const unsigned char *s2 = (const unsigned char *)dst;
    unsigned char c1, c2;

    do {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);

    return c1 - c2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    unsigned char c1 = '\0';
    unsigned char c2 = '\0';

    if (n >= 4) {
        size_t n4 = n >> 2;
        do {
            c1 = (unsigned char)*s1++;
            c2 = (unsigned char)*s2++;
            if (c1 == '\0' || c1 != c2)
                return c1 - c2;
            c1 = (unsigned char)*s1++;
            c2 = (unsigned char)*s2++;
            if (c1 == '\0' || c1 != c2)
                return c1 - c2;
            c1 = (unsigned char)*s1++;
            c2 = (unsigned char)*s2++;
            if (c1 == '\0' || c1 != c2)
                return c1 - c2;
            c1 = (unsigned char)*s1++;
            c2 = (unsigned char)*s2++;
            if (c1 == '\0' || c1 != c2)
                return c1 - c2;
        } while (--n4 > 0);
        n &= 3;
    }

    while (n > 0) {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        if (c1 == '\0' || c1 != c2)
            return c1 - c2;
        n--;
    }
    return 0;
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d < s) {
        // 不重叠，从前往后拷贝
        while (n--) {
            *d++ = *s++;
        }
    } else if (d > s) {
        // 重叠，从后往前拷贝
        d += n;
        s += n;
        while (n--) {
            *(--d) = *(--s);
        }
    }
    // 如果 d 和 s 相等，什么都不做
    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    if (NULL == dest || NULL == src) {
        return NULL;
    }

    void *ret = dest;
    char *d;
    const char *s;
    // 内存不重叠的情况,从低地址开始复制
    if ((dest > (src + n)) || (dest < src)) {
        d = dest;
        s = src;
        while (n--)
            *d++ = *s++;
    }
    // 内存重叠的情况,从高地址开始复制
    else {
        d = (char *)(dest + n - 1); /* offset of pointer is from 0 */
        s = (char *)(src + n - 1);
        while (n--)
            *d-- = *s--;
    }
    return ret;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

#endif
