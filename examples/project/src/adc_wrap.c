#include "board.h"
#include "periph/adc.h"

#include "typedefs.h"

#define RES   ADC_RES_12BIT
#define VREF  (1)

int adc_setup(int line_num) {
     adc_t line = ADC_LINE(line_num);
     if (adc_init(line) < 0)
         return -1;
     else 
         return 0;
}

float adc_measure(int line_num) {
     return adc_sample(ADC_LINE(line_num), RES) / 4096.0 * VREF;
}

void adc_info(void) {
    puts("\n[ADC INFO]:");
    printf("Available ADC devices:               %i\n", ADC_NUMOF);
    printf("ADC line used for VibroSensor: ADC_LINE(%i)\n", VBR_ADC_DEV);
}
