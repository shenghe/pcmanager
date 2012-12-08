#ifndef WILDCMP_INC_
#define WILDCMP_INC_

//////////////////////////////////////////////////////////////////////////

inline int wildcmp(const WCHAR *wild, const WCHAR *string)
{
    const WCHAR *cp = NULL, *mp = NULL;
    while ((*string) && (*wild != L'*')) 
    {
        if ((*wild != *string) && (*wild != L'?')) 
        {
            return 0;
        }
        wild++;
        string++;
    }

    while (*string) 
    {
        if (*wild == L'*') 
        {
            if (!*++wild) 
            {
                return 1;
            }
            mp = wild;
            cp = string+1;
        } 
        else if ((*wild == *string) || (*wild == L'?')) 
        {
            wild++;
            string++;
        } 
        else 
        {
            wild = mp;
            string = cp++;
        }
    }

    while (*wild == L'*') 
    {
        wild++;
    }
    return !*wild;
}

//////////////////////////////////////////////////////////////////////////

#endif WILDCMP_INC_
