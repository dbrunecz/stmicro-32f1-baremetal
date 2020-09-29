
typedef int64_t s64;
typedef uint64_t u64;
typedef int32_t s32;
typedef uint32_t u32;
typedef uint16_t u16;
typedef int8_t s8;
typedef uint8_t u8;

typedef int FILE;
#define stdout  0

#define NULL    ((void *)0)
#define __IO    volatile

#define BIT(x)          (((u32)1) << (x))

#define ARRAY_SIZE(x)   (sizeof(x)/sizeof((x)[0]))

#define MIN(x, y)       ((x) < (y) ? (x) : (y))

int uart_putc(int c, FILE * stream);
int uart_getc(FILE * stream);

int getc(FILE * stream);
int putc(int c, FILE * stream);
int puts(const char *s);
int printf(const char *format, ...);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
int isspace(int c);
int isprint(int c);
void *memset(void *s, int c, size_t n);
unsigned long int strtoul(const char *nptr, char **endptr, int base);
