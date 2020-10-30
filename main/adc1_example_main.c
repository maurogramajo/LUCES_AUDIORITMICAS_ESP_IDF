/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#if CONFIG_IDF_TARGET_ESP32
#include "esp_adc_cal.h"
#endif

#include "soc/rtc.h"

#define CORE0   0
#define CORE1   1

#define LED_BUILTIN 2

#define SAMPLING_RATE   44000
#define NO_OF_SAMPLES   1024

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t canal = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2

static const adc_atten_t atenuacion = ADC_ATTEN_DB_11;


void tFourier(void *a) {
    static const int i2s_num = 0; // i2s port number
    static const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
        .sample_rate = SAMPLING_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_LSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = NO_OF_SAMPLES,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    adc1_config_channel_atten(canal, atenuacion);
    adc1_config_width(ADC_WIDTH_12Bit);
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_adc_mode(ADC_UNIT_1, canal);    //USO ADC1 PORQUE ADC2 NO FUNCIONA CON WIFI
    SET_PERI_REG_MASK(SYSCON_SARADC_CTRL2_REG, SYSCON_SARADC_SAR1_INV);
    i2s_adc_enable(I2S_NUM_0);
    

    while(1) {

    }
}



/*
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

#if CONFIG_IDF_TARGET_ESP32
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
#elif CONFIG_IDF_TARGET_ESP32S2BETA
static const adc_channel_t channel = ADC_CHANNEL_6;     // GPIO7 if ADC1, GPIO17 if ADC2
#endif 
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

#if CONFIG_IDF_TARGET_ESP32
static void check_efuse(void)
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}
#endif
*/
void tTomarMuestra(void * a){

    gpio_pad_select_gpio(LED_BUILTIN);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);
    while(1) {
        gpio_set_level(LED_BUILTIN, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_BUILTIN, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

void app_main(void)
{
    /*
#if CONFIG_IDF_TARGET_ESP32
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();
#endif

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

#if CONFIG_IDF_TARGET_ESP32
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
#endif
*/

    xTaskCreatePinnedToCore(tTomarMuestra, "TomarMuestra", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, 0, CORE0);
/*
    //Continuously sample ADC1
    while (1) {
        uint32_t adc_reading = 0;
        //Multisampling
        unsigned int start_b = xthal_get_ccount();
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        unsigned int end_b = xthal_get_ccount();
        printf("Para tomar %i muestras se necesitaron: %i cicklos de reloj\n", NO_OF_SAMPLES, end_b - start_b);
#if CONFIG_IDF_TARGET_ESP32
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
#elif CONFIG_IDF_TARGET_ESP32S2BETA
        printf("ADC%d CH%d Raw: %d\t\n", unit, channel, adc_reading);
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    */
   //vTaskStartScheduler();
   
}
