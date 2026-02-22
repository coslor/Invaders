#ifndef FIXED_POINT_12_4_INC
#define FIXED_POINT_12_4_INC

/* Define as a struct to keep it as a separate type */
typedef struct {
    int     value;
} fxp12_4;

fxp12_4 int_to_fxp12_4(int n);
fxp12_4 frac_to_fxp12_4(int frac);
fxp12_4 int_frac_to_fxp12_4(int n, int frac);

int fxp12_4_to_int(fxp12_4 fxp);
int fxp12_4_to_frac(fxp12_4 fxp);
int fxp12_4_to_packed_int(fxp12_4 fxp);

fxp12_4 add_frac_to_fxp12_4(fxp12_4 fxp, int frac_value);
fxp12_4 add_fxp12_4_to_fxp12_4(fxp12_4 fxp1, fxp12_4 fxp2);

signed char cmp_fxp12_4(fxp12_4 fxp1, fxp12_4 fxp2);

#pragma compile("fixed_point_12_4.c")

#endif