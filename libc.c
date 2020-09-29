/* Copyright (C) 2020 David Brunecz. Subject to GPL 2.0 */

//#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"

char *hexdigits = "0123456789abcdef";
char *HEXDIGITS = "0123456789ABCDEF";
char *decdigits = "0123456789";

struct print_args {
	int alt;
	int zpad;
	int ladj;
	int blank;
	int sign;

	int width;
	int precision;

	int length;
	int conversion;

	int neg;
	int trunc;
};

int getc(FILE * stream)
{
	return uart_getc(stream);
}

int putc(int c, FILE * stream)
{
	return uart_putc(c, stream);
}

int puts(const char *s)
{
	for (; s && *s; s++)
		putc(*s, stdout);
	return 0;
}

static int _putc(int *cnt, int c, FILE * stream)
{
	(*cnt)++;
	return putc(c, stream);
}

static int _puts(int *cnt, const char *str, int *max)
{
	int i, n = *cnt;

	for (i = 0; *str; i++, str++) {
		if (max && i >= *max)
			break;
		_putc(cnt, *str, stdout);
	}
	return *cnt - n;
}

static void pad(int *cnt, int len, int c)
{
	for (; len > 0; len--)
		_putc(cnt, c, stdout);
}

static void padded_str(int *cnt, struct print_args *p, char *str)
{
	int i, c, len, slen, pad_len;

	slen = strlen(str);
	if (p->neg)
		slen++;

	len = slen > p->precision ? slen : p->precision;

	if (p->trunc)
		len = p->precision;

	pad_len = p->width - len;

	if (!p->ladj)
		pad(cnt, pad_len, p->zpad ? '0' : ' ');

	if (slen < p->precision) {
		if (p->neg && p->zpad)
			_putc(cnt, '-', stdout);
		len = p->precision - slen;
		if (p->neg)
			len--;
		c = (p->zpad || p->conversion == 'x') ? '0' : ' ';
		for (i = 0; i < len; i++)
			_putc(cnt, c, stdout);
		if (p->neg && !p->zpad)
			_putc(cnt, '-', stdout);
	}
	_puts(cnt, str, p->trunc > 0 ? &p->precision : NULL);

	if (p->ladj)
		pad(cnt, pad_len, ' ');
}

static void print_u32(int *cnt, struct print_args *p, u32 v)
{
	int i;
	char b[11];

	for (i = sizeof(b) - 2; v > 0; v /= 10, i--)
		b[i] = '0' + v % 10;
	b[sizeof(b) - 1] = '\0';

	padded_str(cnt, p, &b[i + 1]);
}

static void print_s32(int *cnt, struct print_args *p, s32 v)
{
	int i;
	char b[12];

	if (v < 0) {
		p->neg = 1;
		v = v * -1;
	}
	i = sizeof(b) - 2;
	if (v == 0) {
		b[i] = '0';
		i--;
	} else {
		for (; v > 0; v /= 10, i--)
			b[i] = '0' + v % 10;
	}
	if (p->neg)
		b[i--] = '-';
	b[sizeof(b) - 1] = '\0';

	padded_str(cnt, p, &b[i + 1]);
}

static void print_h32(int *cnt, struct print_args *p, u32 v)
{
	int i;
	char *hd;
	char b[9];

	hd = p->conversion == 'x' ? hexdigits : HEXDIGITS;

	for (i = 0; i < 8; i++)
		b[i] = hd[(v >> (28 - i * 4)) & 0xf];
	b[i] = '\0';
	for (i = 0; b[i] == '0'; i++) ;

	padded_str(cnt, p, &b[i]);
}

void *memset(void *s, int c, size_t n)
{
	u8 *p;
	for (p = s; n; n--, p++)
		*p = c;
	return s;
}

int printf(const char *format, ...)
{
	const char *s;
	va_list args;
	char *t, *e;
	struct print_args p;

	char *_str;
	int _c;
	s32 _s32;
	u32 _u32;

	int count = 0;

	va_start(args, format);

	for (s = format; s && s[0]; s++) {

		if (*s != '%') {
			_putc(&count, *s, stdout);
			continue;
		}
		s++;

		memset(&p, 0, sizeof(p));
		for (t = strchr("#0- +", *s); t; s++, t = strchr("#0- +", *s)) {
			switch (*t) {
			case '#':
				p.alt++;
				break;
			case '0':
				p.zpad++;
				break;
			case '-':
				p.ladj++;
				break;
			case ' ':
				p.blank++;
				break;
			case '+':
				p.sign++;
				break;
			}
		}

		p.width = 0;
		if (*s != '.' && *s != 'h' && *s != 'l') {
			if (*s == '*') {
				p.width = va_arg(args, int);
				s++;
			} else {
				p.width = strtoul(s, &e, 10);
				s = e;
			}
		}

		p.precision = 0;
		if (*s == '.') {
			s++;
			p.precision = strtoul(s, &e, 10);
			s = e;
		}

		p.length = 0;
		if (*s == 'h') {
			s++;
			p.length += 8;
			if (s[1] == 'h') {
				s++;
				p.length += 8;
			}
		} else if (*s == 'l') {
			s++;
			p.length += 32;
			if (s[1] == 'l') {
				s++;
				p.length += 32;
			}
		} else {
			p.length = 32;
		}

		p.conversion = *s;
		switch (p.conversion) {
		case 'd':
		case 'i':
			_s32 = va_arg(args, s32);
			print_s32(&count, &p, _s32);
			break;
		case 'u':
			_u32 = va_arg(args, u32);
			print_u32(&count, &p, _u32);
			break;
		case 'o':
			break;
		case 'p':
		case 'x':
		case 'X':
			_u32 = va_arg(args, u32);
			print_h32(&count, &p, _u32);
			break;
			//case 'e''E''f''F''g''G':
			//    break;
		case 'c':
			_c = va_arg(args, int);
			_putc(&count, _c, stdout);
			break;
		case 's':
			p.trunc = p.precision > 0 ? 1 : 0;
			_str = va_arg(args, char *);
			padded_str(&count, &p, _str);
			break;
		case 'n':
			break;
		case 'm':
			break;
		case '%':
			_putc(&count, '%', stdout);
			break;
		default:
			break;
		}
	}

	va_end(args);
	return count;
}

size_t strlen(const char *s)
{
	size_t n;

	for (n = 0; *s; n++, s++) ;
	return n;
}

char *strcpy(char *dest, const char *src)
{
	char *d;
	for (d = dest; *src; d++, src++)
		*d = *src;
	*d = *src;
	return dest;
}

int strcmp(const char *s1, const char *s2)
{
	for (; *s1 && *s2; s1++, s2++)
		if (*s1 != *s2)
			return *s1 - *s2;
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	for (; s1 && s2 && n; s1++, s2++, n--)
		if (*s1 != *s2)
			return *s1 - *s2;
	return 0;
}

char *strchr(const char *s, int c)
{
	for (; s && *s; s++)
		if (*s == c)
			return (char *)s;
	return NULL;
}

int isspace(int c)
{
	switch (c) {
	case ' ':
	case '\t':
	case '\n':
	case '\r':
	case '\f':
	case '\v':
		return 1;
	}
	return 0;
}

int isprint(int c)
{
	//if (isspace(c))
	if (c == '\n')
		return 1;
	return (c >= ' ' && c <= '~') ? 1 : 0;
}

static u32 x16(u32 v)
{
	return v << 4;
}

static u32 x10(u32 v)
{
	return (v << 3) + (v << 1);
}

static unsigned long int _strtoul(const char *nptr, char **endptr, char *digits,
				  u32(*mul) (u32))
{
	char *d;
	u32 v;

	for (v = 0; *nptr && (d = strchr(digits, *nptr)); nptr++) {
		v = mul(v);
		v += d - digits;
	}
	if (endptr)
		*endptr = (char *)nptr;
	return v;
}

unsigned long int strtoul(const char *nptr, char **endptr, int base)
{
	for (; *nptr && isspace(*nptr); nptr++) ;

	switch (base) {
	case 10:
		return _strtoul(&nptr[0], endptr, decdigits, x10);
	case 16:
		return _strtoul(&nptr[2], endptr, hexdigits, x16);
	case 0:
		if (nptr[0] == '0' && nptr[1] == 'x')
			return _strtoul(&nptr[2], endptr, hexdigits, x16);
		else
			return _strtoul(&nptr[0], endptr, decdigits, x10);
	}
	return ~0;
}
