#pragma once

namespace cv_internal
{

    template<typename T, int shift> struct FixPtCast
    {
        typedef int type1;
        typedef T rtype;
        rtype operator ()(type1 arg) const { return (T)((arg + (1 << (shift-1))) >> shift); }
    };

    template<typename T, int shift> struct FltCast
    {
        typedef T type1;
        typedef T rtype;
        rtype operator ()(type1 arg) const { return arg*(T)(1./(1 << shift)); }
    };

    template<typename T1, typename T2, int cn> int PyrDownVecH(const T1*, T2*, int)
    {
        //   row[x       ] = src[x * 2 + 2*cn  ] * 6 + (src[x * 2 +   cn  ] + src[x * 2 + 3*cn  ]) * 4 + src[x * 2       ] + src[x * 2 + 4*cn  ];
        //   row[x +    1] = src[x * 2 + 2*cn+1] * 6 + (src[x * 2 +   cn+1] + src[x * 2 + 3*cn+1]) * 4 + src[x * 2 +    1] + src[x * 2 + 4*cn+1];
        //   ....
        //   row[x + cn-1] = src[x * 2 + 3*cn-1] * 6 + (src[x * 2 + 2*cn-1] + src[x * 2 + 4*cn-1]) * 4 + src[x * 2 + cn-1] + src[x * 2 + 5*cn-1];
        return 0;
    }

    template<typename T1, typename T2> int PyrDownVecV(T1**, T2*, int) { return 0; }

}
