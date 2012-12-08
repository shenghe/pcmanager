#ifndef MD5_INC_
#define MD5_INC_

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif


/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;

typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} cli_md5_ctx;

extern void cli_md5_init(cli_md5_ctx *ctx);
extern void cli_md5_update(cli_md5_ctx *ctx, void *data, unsigned long size);
extern void cli_md5_final(unsigned char *result, cli_md5_ctx *ctx);

#ifdef __cplusplus
};
#endif

//////////////////////////////////////////////////////////////////////////

#endif  // !MD5_INC_

