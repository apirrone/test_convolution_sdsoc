#ifndef _MAIN4_H_
#define _MAIN4_H_

#define NB_TESTS 100
#define IM_W 100
#define IM_H 67

void sw_naiveConvolution(int width, int height, int border_width, int input[IM_W*IM_H], int conv[IM_W*IM_H]);

#pragma SDS data access_pattern(input:SEQUENTIAL)
void hw_naiveConvolution(int width, int height, int border_width, int input[IM_W*IM_H]);





#endif /* MAIN4 */
