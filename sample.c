#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "base91.h"

int main(void)
{
	int m = 0;
	int k = 0;
	int i = 0,j = 0;
	unsigned char *src = NULL;
	unsigned char *enc = NULL;
	unsigned char *dec = NULL;
	clock_t s,e;
	struct basE91 b;
	srand(time(NULL));

	basE91_init_lib(); // call this function before encoding data

	fprintf(stdout,"This is a sample application\n\n");
	for(i = 12;i <= 48;i += 12)
	{
		m = i * 1024 * 1024;
		src = malloc(m);
		enc = malloc(2 * m);
		dec = malloc(m);
		if (src && enc && dec)
		{
			for(j = 0;j < m;++j)
			{
				*(src + j) = rand() & 0xff;
			}
			basE91_init(&b);
			fprintf(stdout,"Encoding %d MB data\n",m / (1024 * 1024));
			s = clock();
			k = basE91_encode(&b,src,m,enc);
			k += basE91_encode_end(&b,enc + k);
			e = clock();
			fprintf(stdout,"finished. %.3f msec.\n",(double)(e - s)/CLOCKS_PER_SEC);

			basE91_init(&b);
			fprintf(stdout,"Decoding %d MB data\n",k / (1024 * 1024));
			s = clock();
			k = basE91_decode(&b,enc,k,dec);
			k += basE91_decode_end(&b,dec + k);
			e = clock();
			fprintf(stdout,"finished. %.3f msec.\n\n",(double)(e - s)/CLOCKS_PER_SEC);
		}
		free(src);
		free(enc);
		free(dec);
	}
	return 0;
}

