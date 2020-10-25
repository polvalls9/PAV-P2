#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "vad.h"
#include "pav_analysis.h"


const float FRAME_TIME = 10.0F; /* in ms. */
const int TRAMAS_VOZ_NO_DECIDIDAS = 1;     
const int TRAMAS_SILENCIO_NO_DECIDIDAS = 11;
const int N_INICIAL = 13;                       //Numero de ITERACIONES para el CÁLCULO de UMBRAL k0
const float UMBRAL_K0 = 0.9;
const float UMBRAL_K1 = 4.72;


/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT"//,"MAYBE_SILENCE", "MAYBE_VOICE"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
  float sampling_rate;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */

  Features feat;
  feat.p = compute_power(x,N);
  feat.am = compute_am(x, N);
  feat.zcr = compute_zcr(x, N, N/(FRAME_TIME*1e-03)); 
  return feat;
  
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;                           
  vad_data->sampling_rate = rate;                       
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->ko = 0;                                     //Umbral POTENCIA
  vad_data->last_change = 0;                            //Indica ULTIMA TRAMA con V o S
  vad_data->frame = 0;                                  //Número de TRAMA actual
  vad_data->last_state = ST_INIT;                       //Indica ULTIMO ESTADO (V o S)
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  
  VAD_STATE state = vad_data->last_state; 
  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  /* 
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */

  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */
 

  switch (vad_data->state) {

    case ST_INIT:                                                           //CÁLCULO DEL UMBRAL DE POTENCIA
    if (vad_data->frame<N_INICIAL){
        vad_data->ko += pow(10, f.p/10);                                    //CALCULAMOS UMBRAL EN ESCALA LINEAL
       
    }else{

        vad_data->ko = 10*log10(vad_data->ko/N_INICIAL) * UMBRAL_K0;        //CONVERTIMOS a dB
        vad_data->state = ST_MAYBE_SILENCE;
    }
    break;
    

  case ST_SILENCE:
    if (f.p > vad_data->ko)

    vad_data->state = ST_MAYBE_VOICE;
    vad_data->last_state = ST_SILENCE; 
    vad_data->last_change = vad_data->frame;
    
    break;

  case ST_VOICE:
    if (f.p < vad_data->ko)

    vad_data->state = ST_MAYBE_SILENCE;
    vad_data->last_state = ST_VOICE;
    vad_data->last_change = vad_data->frame;
    
    
    break;

  case ST_MAYBE_SILENCE:

    if(f.p > vad_data->ko + UMBRAL_K1){ 
      vad_data->state = ST_VOICE;
    }
    else if ((vad_data->frame - vad_data->last_change) == TRAMAS_SILENCIO_NO_DECIDIDAS){
      vad_data->state = ST_SILENCE;
    }
    break;

  case ST_MAYBE_VOICE:

    if(f.p < vad_data->ko + UMBRAL_K1){
      vad_data->state = ST_SILENCE;
    }
    else if ((vad_data->frame - vad_data->last_change) == TRAMAS_VOZ_NO_DECIDIDAS){
      vad_data->state = ST_VOICE;
    }
    break;
  
  }
  
  
  vad_data->frame++;

  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE)
    return vad_data->state;
  else if (vad_data->state == ST_INIT)
    return ST_SILENCE;
  else
    return ST_UNDEF;

}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}


