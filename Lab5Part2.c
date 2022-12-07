#include <msp430.h> 
unsigned int ADC_ValueTemp;
unsigned int ADC_ValueLight;

int ADCNumber = 1;
#define CALADC_15V_30C  *((unsigned int *)0x1A1A)                 // Temperature Sensor Calibration-30 C
                                                                  // See device datasheet for TLV table memory mapping
#define CALADC_15V_85C  *((unsigned int *)0x1A1C)                 // Temperature Sensor Calibration-High Temperature (85 for Industrial, 105 for Extended)


volatile float temp;
volatile float IntDegF;
volatile float IntDegC;

void initialize_Adc(){
     ADCCTL0 &= ~ADCIFG;//CLEAR FLAG
     ADCMCTL0 &= ADCINCH_0;
     ADCMEM0=0x00000000;
     //ADCAE0=0x00;
     ADCCTL0=0x0000;
     ADCCTL1=0x0000;
}
void ConfigureAdc_temp1(){
    ADCCTL0 &= ~ADCSHT;
    ADCCTL0 |= ADCSHT_8 | ADCON;                                  // ADC ON,temperature sample
    ADCCTL1 |= ADCSHP;                                            // s/w trig, single ch/conv, MODOSC
    ADCCTL1 |=ADCSSEL_2;
    ADCCTL2 &= ~ADCRES;                                           // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;                                          // 12-bit conversion results
    ADCMCTL0 |= ADCINCH_12 | ADCSREF_1;                           // ADC input ch A12 => temp sense
    ADCIE |=ADCIE0;
}

void ConfigureAdc_photosensor(){
    ADCCTL0 &= ~ADCSHT;
    ADCCTL0 |=ADCSHT_8; // 16 CLOCKSON THE BLOCK
    ADCCTL0 |= ADCON; // TURNING ON THE ADC

    ADCCTL1 |=ADCSSEL_2;
    ADCCTL1 |= ADCSHP;//|ADCCONSEQ_1; // SMPE SIGNAL COURCE SAMPLING

    ADCCTL2 &= ~ADCRES; // CLEARING MY ADDRESS
    ADCCTL2 |= ADCRES_2;// RESOLUTION 12 BITS LIKE A BITE
    ADCMCTL0 |=  ADCINCH_3 | ADCSREF_1 ;
    ADCIE |= ADCIE0;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // TURN ON GPIOS

    // PACKING MY PORTS
    P1SEL0 = BIT3;
    P1SEL1 = BIT3;

    while(1)
    {

      if (ADCNumber == 1 )// this is to flip and flop so it reads one ADC at a time
      {

          initialize_Adc(); // clears and initializes the ADC
          ConfigureAdc_photosensor(); // starts up the photosensor ADC and its parameters
          ADCCTL0 |= ADCENC | ADCSC|ADCMSC; //enable and start conversion
          while((ADCCTL0 & ADCIFG) == 0);
          _delay_cycles(200000);

          ADC_ValueLight = ADCMEM0;
          ADCNumber = 12;
      }
      else
      {
          initialize_Adc(); // clears and initializes the ADC
          PMMCTL0_H = PMMPW_H;                                          // Unlock the PMM registers
          PMMCTL2 |= INTREFEN | TSENSOREN | REFVSEL_0;
          ConfigureAdc_temp1(); // Configurs the ADC for temp
          ADCCTL0 |= ADCENC | ADCSC | ADCMSC; //enable and start conversion
          while((ADCCTL0 & ADCIFG) == 0);
          _delay_cycles(200000);

          temp = ADCMEM0;
          IntDegC = (temp-CALADC_15V_30C)*(85-30)/(CALADC_15V_85C-CALADC_15V_30C)+30; // conversion
          IntDegF = 9*IntDegC/5+32;
          ADCNumber = 1;
      }
    }
}


