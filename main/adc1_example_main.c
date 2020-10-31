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
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#if CONFIG_IDF_TARGET_ESP32
#include "esp_adc_cal.h"
#endif

#include "soc/rtc.h"
#include "driver/i2s.h"
#include "soc/syscon_reg.h"

#define CORE0   0
#define CORE1   1

#define LED_BUILTIN 2

#define FREQ_MUESTREO   40000
#define NUM_MUESTRAS   1024

//static esp_adc_cal_characteristics_t *adc_chars;
#define V_REF   1100
static const adc_channel_t canal = ADC_CHANNEL_0;  //ahora es pin VP   //GPIO34 if ADC1, GPIO14 if ADC2

//static const adc_atten_t atenuacion = ADC_ATTEN_DB_11;

QueueHandle_t qI2sReader;
TaskHandle_t readerTaskHandle;

i2s_config_t adcI2SConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = FREQ_MUESTREO,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = NUM_MUESTRAS,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

void i2sReaderTask(void *a) {
    i2s_driver_install(I2S_NUM_0, &adcI2SConfig, 4, &qI2sReader);
    i2s_set_adc_mode(ADC_UNIT_1, canal);
    //SET_PERI_REG_MASK(SYSCON_SARADC_CTRL2_REG, SYSCON_SARADC_SAR1_INV);
    i2s_adc_enable(I2S_NUM_0);   

    int i2s_read_len = NUM_MUESTRAS*sizeof(uint16_t);
    char i2s_read_buff[NUM_MUESTRAS];
    //uint16_t i2s_read_buff[NUM_MUESTRAS];

    while(1) {
        i2s_event_t evt;
        if (xQueueReceive(qI2sReader, &evt, portMAX_DELAY) == pdPASS)
        {
            if (evt.type == I2S_EVENT_RX_DONE)
            {
                size_t bytesRead = 0;
                do
                {
                    // read data from the I2S peripheral
                    uint8_t i2sData[NUM_MUESTRAS];
                    // read from i2s
                    i2s_read(I2S_NUM_0, i2sData, NUM_MUESTRAS, &bytesRead, 10);
                    // process the raw data
                    uint16_t *rawSamples = (uint16_t *)i2sData;
                    for (int i = 0; i < bytesRead / 2; i++)
                    {
                        addSample((2048 - (rawSamples[i] & 0xfff)) * 15);
                    }
                } while (bytesRead > 0);
            }
        }
    }
}

void tBlinky(void * a){

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
    xTaskCreatePinnedToCore(tBlinky, "Blinky", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, 0, CORE0);
    //xTaskCreatePinnedToCore(tFourier, "FFT", configMINIMAL_STACK_SIZE*5, NULL, tskIDLE_PRIORITY+1, 0, CORE0);
    // start a task to read samples from the ADC
    
    xTaskCreatePinnedToCore(i2sReaderTask, "i2s Reader Task", 4096, NULL, tskIDLE_PRIORITY+1, &readerTaskHandle, CORE0);
}
