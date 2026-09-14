#ifndef SIZE_T_H
#define SIZE_T_H
    
#if defined(__GNUC__) || defined(__clang__)
    typedef __SIZE_TYPE__ size_tz;
#else 
    typedef unsigned long long size_tz;
#endif

#endif
