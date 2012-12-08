// Md5.h: interface for the Md5 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MD5_H
#define _MD5_H

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

class KMd5
{
public:
	KMd5();
	virtual ~KMd5();

	void md5_update(uint8 *input, uint32 length );
	void md5_finish(uint8 digest[16] );
	

private:
	uint32 total[2];
    uint32 state[4];
    uint8 buffer[64];

	void md5_process(uint8 data[64] );
};

#endif /* md5.h */
