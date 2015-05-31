#include "proximity.h"

#include "ch.h"
#include "hal.h"
#include "tcrt1000.h"
#include "analogic.h"

#define PWM_CLK_FREQ 42000000
#define PWM_CYCLE 4200
#define TCRT1000_DC 0.071               //Duty cycle for IR sensors
#define COUNTER_HIGH_STATE 0x0096       //150 CLK cycle
#define COUNTER_LOW_STATE  0x0FA0       //4000 CLK cycle
#define WATCHDOG_RELOAD_VALUE 0x0028    //40 in hexadecimal
#define NUM_IR_SENSORS 13

static void proximity_pwm_cb(PWMDriver *pwmp)
{
    (void)pwmp;
    IWDG->KR = 0xAAAA;      //Resets watchdog counter
}

static const PWMConfig pwmcfg_proximity = {
    PWM_CLK_FREQ,                                   /* 42MHz PWM clock frequency.  */
    PWM_CYCLE,                                      /* PWM cycle is 10kHz.   */
    proximity_pwm_cb,
    {
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_DISABLED, NULL},
    {PWM_OUTPUT_DISABLED, NULL}
    },
    /* HW dependent part.*/
    0,                  // TIMx_CR2 value
    0                   // TIMx_DIER value
};

int proximity_change_adc_trigger(void) {
    if (STM32_TIM8->CCR[0] == COUNTER_HIGH_STATE) {
        STM32_TIM8->CCR[0] = COUNTER_LOW_STATE;
        return FALSE;
    }
    else {
        STM32_TIM8->CCR[0] = COUNTER_HIGH_STATE;
        return TRUE;
    }
}

void proximity_get(float* proximity)
{
    int32_t value[NUM_IR_SENSORS];
    int i;
    analog_get_proximity(value);

    for (i = 0; i < NUM_IR_SENSORS; i++){
        proximity[i] = (float) value[i];
    }
}

void proximtiy_init(void) {
    STM32_TIM8->CCR[0] = COUNTER_HIGH_STATE;        //Compare value
    STM32_TIM8->CCMR1 |= TIM_CCMR1_OC1M_0;          //Output compare mode, sets output to high on match
    STM32_TIM8->CCMR1 &= ~(TIM_CCMR1_OC1PE);        //Disables preload
    STM32_TIM8->CCER &= ~(TIM_CCER_CC1P);           //active high polarity
    STM32_TIM8->CCER |= TIM_CCER_CC1E;              //enable output
}


void proximity_start(void) {
    pwmStart(&PWMD8, &pwmcfg_proximity);

    /*Watchdog to stop the PWM in case of malfunction*/
    /*Watchdog clock frequency is 32kHz*/
    IWDG->RLR = (IWDG_RLR_RL | WATCHDOG_RELOAD_VALUE);
    IWDG->KR |= 0xCCCC;             //Starts Watchdog countdown

    pwmEnableChannel(&PWMD8, 0, (pwmcnt_t) (PWM_CYCLE * TCRT1000_DC));

    analogic_start(0,0,1);
}