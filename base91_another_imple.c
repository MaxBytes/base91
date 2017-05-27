
/*

base91_another_imple.c

Overview:
	Another implementation of basE91 encoding / decoding algorithm.

Description:
	Original code (http://base91.sourceforge.net/) is not optimized for speed.
	This implementation gives slightly fast encoding speed.

Note:
	This code assumes little endian.
	For big endian, this code needs to be modified.

License:
	MIT license. See below.

*/

/*

MIT License

Copyright (c) 2016 MaxBytes

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "base91.h"

unsigned char const encodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!#$%&()*+,./:;<=>?@[]^_`{|}~\"";

int const decodeTable[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, 62, 90, 63, 64, 65, 66, -1, 67, 68, 69, 70, 71, -1, 72, 73,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 74, 75, 76, 77, 78, 79,
	80,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 81, -1, 82, 83, 84,
	85, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 86, 87, 88, 89, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};


// Partially encoded string is stored into following table prior to actual encoding.
unsigned int pre_encoded_b91[16*1024];

unsigned int pre_decoded_b91[65536];

// This function must be called only once between program startup and termination.
// Otherwise, encoding / decoding function generates wrong result.
void basE91_init_lib(void)
{
	unsigned int u = 0;
	for(u = 0;u <= 0x3fff;++u)
	{
		if ((u & 0x1fff) > 88)
		{
			pre_encoded_b91[u] = (encodeTable[(u & 0x1fff) / 91] << 8)|encodeTable[(u & 0x1fff) % 91];
			pre_decoded_b91[0xffff & pre_encoded_b91[u]] = u & 0x1fff;
		}
		else
		{
			pre_encoded_b91[u] = (encodeTable[(u & 0x3fff) / 91] << 8)|encodeTable[(u & 0x3fff) % 91];
			pre_decoded_b91[0xffff & pre_encoded_b91[u]] = u & 0x3fff;
		}
	}
	return;
}

// Initialise basE91 object b.
// For each basE91 object b, this function must be called only once before encode / decode data.
void basE91_init(struct basE91 *b)
{
	b->q = 0;
	b->bits = 0;
	b->val = -1;
}

// Encode (len) bytes data in the input buffer i into output buffer o.
unsigned long basE91_encode(struct basE91 *b,unsigned char const *i,unsigned long len,unsigned char *o)
{
	unsigned long long u = 0;
	int s = 0,t = 0;
	unsigned long cnt = 0;

	// Process each 32 bits word in the input buffer. So, we need at least 4 bytes.
	while(len >= 4)
	{
		u = b->q | ((unsigned long long)(*((unsigned int*)i)) << b->bits);

		// can take 14 bits ?
		s = (u & 0x1fff) <= 88;
		t = ((u >> (13 + s)) & 0x1fff) <= 88;

		// Store encoded string (4 characters long) into output buffer.
		*((unsigned int*)o) = pre_encoded_b91[u & 0x3fff] | (pre_encoded_b91[((u >> (13 + s)) & 0x3fff)] << 16);

		// 32 bits word will be encoded acording to following step
		// first, 13 or 14 bits are taken and encoded. remianing 19 or 18 bits will be encoded next step
		// second, 13 or 14 bits are taken and encoded. remaining 4,5 or 6 bits remains unprocessed.
		// as shown above, at least 26 (= 13 + 13) bits are taken.
		// if first or/and second step takes 14 bits,
		// then 27(= 13 + 14) or 28 (= 14 + 14) bits are taken and encoded.
		// finally, 26,27 or 28 bits will be encoded.

		b->q = u >>= (26 + s + t); // (26 + s + t) will be 26,27 or 28
		b->bits += (32 - 26 - s - t); // 

		o += 4;
		i += 4;
		len -= 4;
		cnt += 4;

		if (b->bits > 13)
		{
			// now we can process remaining b->bits.

			*((unsigned short*)o) = pre_encoded_b91[b->q & 0x3fff];
			if ((b->q & 0x1fff) > 88)
			{
				b->q >>= 13;
				b->bits -= 13;
			}
			else
			{
				b->q >>= 14;
				b->bits -= 14;
			}
			o += 2;
			cnt += 2;
		}
	}

	// process remaining bytes (at most 3 bytes)
	while(len--)
	{
		b->q |= *i++ << 8;
		if (b->bits > 5)
		{
			*((unsigned short*)o) = pre_encoded_b91[b->q & 0x3fff];
			if ((b->q & 0x1fff) > 88)
			{
				b->q >>= 13;
				b->bits -= 5;
			}
			else
			{
				b->q >>= 14;
				b->bits -= 6;
			}
			o += 2;
			cnt += 2;
		}
		else b->bits += 8;
	}
	return cnt;
}

// process remaining bits and finalise.
unsigned long basE91_encode_end(struct basE91 *b,unsigned char *o)
{
	unsigned long cnt = 0;
	if (b->bits)
	{
		*o++ = encodeTable[b->q % 91];
		cnt++;
		if (b->bits > 7 || b->q > 90)
		{
			*o++ = encodeTable[b->q / 91];
			cnt++;
		}
	}
	b->q = 0;
	b->bits = 0;
	b->val = -1;
	return cnt;
}

// Decode (len) bytes basE91 string in the input buffer i into output buffer o.
unsigned long basE91_decode(struct basE91 *b,unsigned char const *i,unsigned long len,unsigned char *o)
{
	register unsigned x = 0;
	unsigned long long u = 0;
	unsigned int x1 = 0,x2 = 0,x3 = 0,x4 = 0;
	int s1 = 0,s2 = 0,s3 = 0,s4 = 0;
	unsigned long cnt = 0;
	int d = 0;

	// We require at least 8 bytes to process two 32bits words at once.
	while(len >= 8)
	{
		x = *((unsigned int*)i);
		x1 = pre_decoded_b91[x & 0xffff];
		x2 = pre_decoded_b91[(x >> 16) & 0xffff];

		x = *((unsigned int*)(i + 4));
		x3 = pre_decoded_b91[x & 0xffff];
		x4 = pre_decoded_b91[(x >> 16) & 0xffff];

		s1 = (x1 & 0x1fff) <= 88;
		s2 = (x2 & 0x1fff) <= 88;
		s3 = (x3 & 0x1fff) <= 88;
		s4 = (x4 & 0x1fff) <= 88;

		u = x4; u <<= (13 + s3);
		u |= x3; u <<= (13 + s2);
		u |= x2; u <<= (13 + s1);
		u |= x1; u <<= b->bits;
		u |= b->q;
		b->bits += (52 + s4 + s3 + s2 + s1);

		*((unsigned int*)o) = u;
		b->q = u >>= 32; b->bits -= 32;
		o += 4; cnt += 4;

		if (b->bits == 32)
		{
			b->q = 0;
			b->bits = 0;
			*((unsigned int*)o) = b->q;
			b->q >>= 32; b->bits -= 32;
			o += 4; cnt += 4;
		}

		while(b->bits >= 8)
		{
			*o++ = b->q;
			b->q >>= 8;
			b->bits -= 8;
			cnt++;
		}
		i += 8; len -= 8;
	}

	// Process remaining bytes
	while(len--)
	{
		if (-1 == (d = decodeTable[*i++]))
			continue;
		if (b->val == -1)
		{
			b->val = d;
			continue;
		}
		b->val += d*91;
		b->q |= b->val << b->bits;
		b->bits += (13 + ((b->val & 0x1fff) <= 88));
		while(b->bits >= 8)
		{
			*o++ = b->q;
			b->q >>= 8;
			b->bits -= 8;
			cnt++;
		}
		b->val = -1;
	}
	return cnt;
}

unsigned long basE91_decode_end(struct basE91 *b,unsigned char *o)
{
	unsigned long cnt = 0;
	if (b->val != -1)
	{
		*o++ = b->q | (b->val << b->bits);
		cnt++;
	}
	b->q = 0;
	b->bits = 0;
	b->val = -1;
	return cnt;
}
