#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

float acc_x[1600];
float acc_y[1600];
float acc_z[1600];

// THPC_sensor
float temp_generator(){
  // Random float entre 5.0 y 30.0
  float temp = ((30.0 - 5.0)*((float)rand() / RAND_MAX)) + 5.0;
  return temp;
}

int hum_generator(){
  // Random int entre 30 y 80
  int hum = rand() % 51 + 30;
  return hum;
}

int pres_generator(){
  // Random int entre 1000 y 1200
  int pres = rand() % 201 + 1000;
  return pres;
}

float co_generator(){
  // Random float entre 30.0 y 200.0
  float co = ((200.0 - 30.0)*((float)rand() / RAND_MAX)) + 30.0;
  return co;
}


// Batt_sensor
int value_generator(){
  // Random int entre 1 y 100
  int value = rand() % 100 + 1;
  return value;
}


// Aceloremeter_kpi
float amp_x_generator(){
  // Random float entre 0.0059 y 0.12
  float amp_x = ((0.12 - 0.0059)*((float)rand() / RAND_MAX)) + 0.0059;
  return amp_x;
}

float amp_y_generator(){
  // Random float entre 0.0041 y 0.11
  float amp_y = ((0.11 - 0.0041)*((float)rand() / RAND_MAX)) + 0.0041;
  return amp_y;
}

float amp_z_generator(){
  // Random float entre 0.008 y 0.15
  float amp_z = ((0.15 - 0.008)*((float)rand() / RAND_MAX)) + 0.008;
  return amp_z;
}

float RMS_generator(){
  // Genera el valor RMS de amp
  float x = amp_x_generator();
  float y = amp_y_generator();
  float z = amp_z_generator();
  float RMS = sqrt(pow(x,2) + pow(y,2) +pow(z,2));
  return RMS;
}

float frec_x_generator(){
  // Random float entre 29.0 y 31.0
  float frec_x = ((31.0 - 29.0)*((float)rand() / RAND_MAX)) + 29.0;
  return frec_x;
}

float frec_y_generator(){
  // Random float entre 59.0 y 61.0
  float frec_y = ((61.0 - 59.0)*((float)rand() / RAND_MAX)) + 59.0;
  return frec_y;
}

float frec_z_generator(){
  // Random float entre 89.0 y 91.0
  float frec_z = ((91.0 - 89.0)*((float)rand() / RAND_MAX)) + 89.0;
  return frec_z;
}