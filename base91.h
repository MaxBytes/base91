#ifndef BASE91_H
#define BASE91_H

#include <stdint.h>

typedef struct basE91
{
	unsigned long q;
	unsigned int bits;
	int val;
} BASE91;

#ifdef __cplusplus
extern "C" {
#endif

extern void basE91_init_lib(void);

extern void basE91_init(struct basE91 *b);

extern unsigned long basE91_encode(struct basE91 *b,unsigned char const *i,unsigned long len,unsigned char *o);

extern unsigned long basE91_encode_end(struct basE91 *b,unsigned char *o);

extern unsigned long basE91_decode(struct basE91 *b,unsigned char const *i,unsigned long len,unsigned char *o);

extern unsigned long basE91_decode_end(struct basE91 *b,unsigned char *o);

#ifdef __cplusplus
}
#endif

#endif