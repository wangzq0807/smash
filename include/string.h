#ifndef __STRING_H__
#define __STRING_H__
#include "sys/types.h"
#include "stddef.h"

static inline void memcpy(void *dest, const void *src, size_t size) {
    while (size--) {
        ((uint8_t*)dest)[size] = ((uint8_t*)src)[size];
    };
}

static inline void memset(void *dest, int value, size_t size) {
    while (size--) {
        ((char*)dest)[size] = value & 0XFF;
    }
}

static inline int strlen(const char *str) {
    int ret = 0;
    for (; str[ret]; ret++) ;
    return ret;
}

static inline int strcmp(const char *s1, const char *s2) {
    while ( *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    int diff = s1[0] - s2[0];
    if (diff > 0)
        return 1;
    else if(diff < 0)
        return -1;
    else
        return 0;
}

static inline int strncmp(const char *s1, const char *s2, size_t n) {
    while ( *s1 && (*s1 == *s2) && n-- > 0) {
        s1++;
        s2++;
    }
    if (n == 0) return 0;

    int diff = s1[0] - s2[0];
    if (diff > 0)
        return 1;
    else if(diff < 0)
        return -1;
    else
        return 0;
}

static inline const char *strstr(const char *str, const char *need) {
    const char *itr = need;
    while (*str) {
        if (*str == *need) {
            while (*str && *itr && (*str == *itr)) {
                str++;
                itr++;
            }
            const int diff = itr - need;
            if (*itr == 0)
                return str -= diff;
            else
                itr = need;
        }
        else {
            str++;
        }
    }
    return str;
}

static inline const char *strcpy(char *dst, const char *src) {
    const char *ret = dst;
    while (*src) {
        *dst++ = *src++;
    }
    *dst = 0;
    return ret;
}

static inline char *strsep(char **str, const char *delim) {
    char *ret = *str;
    char *itr = *str;
    const char *diter = delim;
    for (; *itr; ++itr) {
        int isdelim = 0;
        for (; *diter; ++diter) {
            if (*itr == *diter) {
                isdelim = 1;
                break;
            }
        }
        diter = delim;
        if (isdelim) {
            if (itr == ret)
                ret++;
            else {
                *itr++ = 0;
                break;
            }
        }
    }
    *str = itr;
    if (*ret)
        return ret;
    else
        return NULL;
}

static inline char *strim(char *str, const char *trim) {
    char *ret = str;
    char *itr = str;
    char *end = NULL;
    const char *itrim = trim;
    for (; *itr; ++itr, ++ret) {
        for (; *itrim && (*itr != *itrim); ++itrim);
        if (*itrim == 0)
            break;
        itrim = trim;
    }

    for (; *itr; ++itr) {
        for (; *itrim && (*itr != *itrim); ++itrim);
        if (*itrim == 0)
            end = itr;
        itrim = trim;
    }
    if (end != NULL)
        *(++end) = 0;
    return ret;
}

static inline char *strcat(char *dst, const char *src) {
    char *itr = dst;
    while (*itr++);
    strcpy(itr-1, src);
    return dst;
}

#endif