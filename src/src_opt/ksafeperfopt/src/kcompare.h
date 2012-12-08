#pragma once
#ifndef KCOMPARE_H
#define KCOMPARE_H

#define KPFW_COMPARE_RESULT_GREAT				 1
#define KPFW_COMPARE_RESULT_EQUAL				 0
#define KPFW_COMPARE_RESULT_LESS				-1
#define KPFW_COMPARE_RESULT_ERROR				-2


class KCompare
{
public:
    static int StringNoCase(ATL::CString str1, ATL::CString str2);

    template <typename T>
    static int Compare(const T& a, const T& b)
    {
        if(a == b)
        {
            return KPFW_COMPARE_RESULT_EQUAL;
        }
        else if(a < b)
        {
            return KPFW_COMPARE_RESULT_LESS;
        }
        else
        {
            return KPFW_COMPARE_RESULT_GREAT;
        }
    }

    template <typename T>
    static int PointerValueCompare(const T* a, const T* b)
    {
        return Compare<T>(*a, *b);
    }
};

#endif //KCOMPARE_H