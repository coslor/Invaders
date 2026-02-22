#include "fixed_point_12_4.h"

inline void _packed_int_to_fxp12_4_ptr(fxp12_4 *fxp, int n) {
    *((int*)&fxp) = n;
}

/* The "packed" int == integer << 4 + fraction */
inline int fxp12_4_to_packed_int(fxp12_4 fxp) {
    return *((int*)&fxp);
}

inline fxp12_4 _packed_int_to_fxp12_4(int packed_int) {
    fxp12_4 fxp;
    fxp = *((fxp12_4*)&packed_int);
    return fxp;
}

inline fxp12_4 int_to_fxp12_4(int n) {
    return int_frac_to_fxp12_4(n,0);
}

/* frac is a ratio of frac/16, so 8=.5, 4=.25, 16=1.0,32=2.0, etc. */
inline fxp12_4 frac_to_fxp12_4(int frac) {
    return int_frac_to_fxp12_4(0, frac);
}

inline fxp12_4 int_frac_to_fxp12_4(int n, int frac) {
    __assume(frac < 16);
    fxp12_4 fxp = _packed_int_to_fxp12_4(n<<4 + frac);
    return fxp;
}

/* Truncates */
inline int fxp12_4_to_int (fxp12_4 fxp) {
    return fxp12_4_to_packed_int(fxp) >> 4;
}

inline int fxp12_4_to_frac(fxp12_4 fxp) {
    return fxp12_4_to_packed_int(fxp) & 0b1111;
}

inline fxp12_4 add_frac_to_fxp12_4(fxp12_4 fxp, int frac_value) {
//    __assume(frac_value < 16);
    return (_packed_int_to_fxp12_4(fxp12_4_to_packed_int(fxp) + frac_value));
}

inline fxp12_4 add_fxp12_4_to_fxp12_4(fxp12_4 fxp1, fxp12_4 fxp2) {
    fxp12_4 fxp3 = _packed_int_to_fxp12_4(fxp12_4_to_packed_int(fxp1) + fxp12_4_to_packed_int(fxp2));
    return fxp3;
}

inline fxp12_4 add_int_to_fxp12_4(fxp_12_4 fxp, int n) {
    return add_fxp12_to_fxp_12(fxp, int_to_fxp12(n));
}

signed char cmp_fxp12_4(fxp12_4 fxp1, fxp12_4 fxp2) {
     int pack1 = fxp12_4_to_packed_int(fxp1);
     int pack2 = fxp12_4_to_packed_int(fxp2);
     if (pack1 > pack2) {
        return 1;
     } else if (pack1 < pack2) {
        return -1;
     } else {
        return 0;
     }
}

inline fxp12_4 float_to_fxp12_4(float f) {

    return int_frac_to_fxp12_4((int)f, (int)(f - (int)f)*16);
}

inline float fxp12_4_to_float(fxp12_4 fxp) {
    return (float)(fxp12_4_to_int(fxp) + (fxp12_4_to_frac(fxp) / 16));
}

inline char* fxp12_4_to_str(char buffer[], fxp12_4 fxp) {
    sprintf(buffer, FXP12_4_FORMAT_STR, fxp12_4_to_int(fxp), fxp12_4_to_frac(fxp));
    return buffer;
}
