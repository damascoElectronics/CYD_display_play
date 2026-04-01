#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "bsp_pinout.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/** @brief Number of samples to average for the LDR reading */
#define LDR_SAMPLES  4  
/** @brief Alpha value for the Exponential Moving Average filter */
#define EMA_ALPHA    0.3f

static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t         cali_handle = NULL;
static float                     ema_value = 2048.0f;

esp_err_t bsp_ldr_init(void) {
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id  = BSP_LDR_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t ret = adc_oneshot_new_unit(&unit_cfg, &adc_handle);
    if (ret != ESP_OK) {
        printf("ERROR adc_oneshot_new_unit: %s\n", esp_err_to_name(ret));
        return ret;
    }

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, BSP_LDR_CHANNEL, &chan_cfg));

    adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id  = BSP_LDR_UNIT,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_cali_create_scheme_line_fitting(&cali_cfg, &cali_handle);

    int test_raw = 0;
    adc_oneshot_read(adc_handle, BSP_LDR_CHANNEL, &test_raw);
    printf("TEST raw inmediato: %d\n", test_raw);
    
    printf("LDR init OK\n");

    return ESP_OK;
}

// -- Internal functions first --

// Raw ADC reading
static int ldr_read_raw(void) {
    if (adc_handle == NULL) {
        return 0;
    }
    int raw = 0;
    adc_oneshot_read(adc_handle, BSP_LDR_CHANNEL, &raw);
    return raw;
}

// Average of N samples
static int ldr_read_avg(void) {
    int sum = 0;
    for (int i = 0; i < LDR_SAMPLES; i++) {
        sum += ldr_read_raw();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    return sum / LDR_SAMPLES;
}

// -- Public API --

int bsp_ldr_read(void) {
    int avg = ldr_read_avg();
    ema_value = EMA_ALPHA * avg + (1.0f - EMA_ALPHA) * ema_value;
    return (int)ema_value;
}

int bsp_ldr_read_mv(void) {
    if (cali_handle == NULL) return 0;
    int mv = 0;
    adc_cali_raw_to_voltage(cali_handle, bsp_ldr_read(), &mv);
    return mv;
}