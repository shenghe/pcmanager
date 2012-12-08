#ifndef ACF_MISC_INC_
#define ACF_MISC_INC_

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

int __fastcall StrLen(const char *Str1);
char * __fastcall StrCpy(char *Dest, const char *Source);
char * __fastcall StrLCopy(char *Dest,const char *Source, int MaxLen);
int __fastcall StrCmp(const char *Str1, const char *Str2);
int __fastcall StrICmp(const char *Str1, const char *Str2);
char * __fastcall StrChr(const char *Str1, const char Chr);
char * __fastcall StrRChr(const char *Str1, const char Chr);
void __fastcall MemCpy(void *Dst, const void *Src, unsigned int tSize);
void __fastcall MemMove(void *Dst, const void *Src, unsigned int tSize);
int __fastcall MemCmp(const void *Dst, const void *Src, unsigned int tSize);
void __fastcall MemSet(void *Buffer, char ch1, size_t Count);
char * __fastcall StrCat(char *DestStr, const char *SrcStr);

#ifdef __cplusplus
};
#endif

//////////////////////////////////////////////////////////////////////////

#endif  // !ACF_MISC_INC_
