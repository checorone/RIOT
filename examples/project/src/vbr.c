#include <stdio.h>

#include "mutex.h"

#include "typedefs.h"

extern int   adc_setup(int line_num);
extern float adc_measure(int line_num);

extern int trace_vbr_on;

void set_data_vbr(float vibration);
void get_data_vbr(float * vibration);
void clear_data_vbr(void);

vbr_data_t vbr_data;

static mutex_t mutex = MUTEX_INIT;

void init_vbr(void) {
    puts("\n[VBR] [INFO]: Starting initialization...");
    if (adc_setup(VBR_ADC_DEV) == -1) {
        printf("[VBR] [ERROR]: ADC line (%d) creation failed. Aborting...\n", VBR_ADC_DEV);
        return;
    }
    printf("[VBR] [INFO]: Successfully initiated ADC line (%d).\n", VBR_ADC_DEV);
    printf("[VBR] [INFO]: Successfully initiated VibrationSensor on ADC line (%d).\n", VBR_ADC_DEV);
}

void process_vbr(void) {
    float sample = adc_measure(VBR_ADC_DEV);
    if (sample < 0) {
        if(trace_vbr_on)
            printf("[VBR] [ERROR]: Failed to measure vibration on ADC line (%d)\n", VBR_ADC_DEV);
    }
    else {
        set_data_vbr(sample);
        if(trace_vbr_on)
            printf("[VBR] [DATA]: Vibration level = (%f)\n", sample);
    }
}

void set_data_vbr(float vibration) {
    mutex_lock(&mutex);
    if(vibration > vbr_data.vibration)
        vbr_data.vibration = vibration;
    mutex_unlock(&mutex);
}

void get_data_vbr(float * vibration) {
    mutex_lock(&mutex);
    *vibration = vbr_data.vibration;
    mutex_unlock(&mutex);
}

void clear_data_vbr(void) {
    vbr_data.vibration = 0;
}
