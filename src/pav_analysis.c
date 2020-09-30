#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {

float pot=0;

for(int i=0; i<N; i++){

    pot = pot + (x[i] * x[i]);
}
pot= pot / N;
pot = 10 * log10(pot);

return pot;
}

float compute_am(const float *x, unsigned int N) {
float xabs = 0,am;

for(int i=0; i<N; i++){

    xabs = xabs + fabs(x[i]);
}

am = xabs / N;
return am;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
float zcr = 0;

for(int i=1; i<N; i++){
    if((x[i]>0 && x[i-1]<=0) || (x[i]<0 && x[i-1]>=0)){

            zcr = zcr + 1;
    }
}
zcr = zcr*fm/(2*(N-1));

return zcr;
}