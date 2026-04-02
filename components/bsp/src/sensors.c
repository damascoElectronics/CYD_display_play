/**
 * @file sensors.c
 * @brief Implementations for board sensors (e.g., LDR ADC reads, conversions, filtering).
 */
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

/** @brief ADC Oneshot unit handle used for LDR readings */
static adc_oneshot_unit_handle_t adc_handle = NULL;
/** @brief ADC Calibration handle to convert raw readings to voltage */
static adc_cali_handle_t cali_handle = NULL;
/** @brief State variable to keep track of the Exponential Moving Average (EMA) */
static float ema_value = 2048.0f;

/**
 * @brief Initializes the LDR ADC unit, channel, and calibration scheme.
 * 
 * @return esp_err_t ESP_OK if setup completes successfully, or an error code otherwise.
 */
esp_err_t bsp_ldr_init(void)
{
    // Declare and populate an ADC unit configuration struct
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id  = BSP_LDR_UNIT,    // Set the target ADC unit to the BSP defined unit
        .ulp_mode = ADC_ULP_MODE_DISABLE, // Explicitly disable ultra-low power mode for this component
    };
    // Initialize the ADC unit using the configuration struct and Conditional check if unit initialization reported an error
    if (adc_oneshot_new_unit(&unit_cfg, &adc_handle) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Declare and populate an ADC channel configuration struct
    adc_oneshot_chan_cfg_t chan_cfg = { 
        .atten    = ADC_ATTEN_DB_12, // Setup signal attenuation to enable max read voltage (12dB)
        .bitwidth = ADC_BITWIDTH_12, // Setup precision bitwidth explicitly to 12 bits resolution
    };
    
    // Apply config to channel and assert success
    if (adc_oneshot_config_channel(adc_handle, BSP_LDR_CHANNEL, &chan_cfg) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Declare and populate calibration scheme setup struct
    adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id  = BSP_LDR_UNIT,    // Set calibration scheme unit matching LDR configuration
        .atten    = ADC_ATTEN_DB_12, // Set calibration scheme attenuation matching channel config
        .bitwidth = ADC_BITWIDTH_12, // Set calibration scheme bitwidth matching channel config
    };
    // Initialize firmware mapping to calibrate raw ADC output
    adc_cali_create_scheme_line_fitting(&cali_cfg, &cali_handle); 
    // Return final success confirmation to the caller
    return ESP_OK;                   
}

/**
 * @brief Gets a single raw reading directly from the ADC unit.  Internal private definitions section marker
 * 
 * @return int Raw ADC value read (0-4095). Returns 0 if uninitialized.
 */
static int ldr_read_raw(void)
{
    // Sanity-check that ADC module actually configured successfully
    if (adc_handle == NULL) 
    {
        // Fail safe: return zero to avoid crashing upon missing resource
        return 0;
    }
    // Construct internal variable holding hardware return output
    int raw = 0;
    // Command ADC peripheral driver to fetch single point value
    adc_oneshot_read(adc_handle, BSP_LDR_CHANNEL, &raw);
    // Transport localized variable output up the call chain
    return raw;
}

/**
 * @brief Samples the LDR multiple times and calculates an average.
 * 
 * Takes `LDR_SAMPLES` samples with a small 2ms delay between each to provide
 * a slightly filtered baseline.
 * 
 * @return int The averaged raw value.
 */
static int ldr_read_avg(void)
{
    // Ready memory cache for ongoing cumulative additions
    int sum = 0;
    // Initiate sequential fetch execution loop
    for (int i = 0; i < LDR_SAMPLES; i++) 
    { 
        // Trigger explicit localized raw grab, adding onto pool
        sum += ldr_read_raw();
        // Stash thread execution back onto OS dispatcher queue for 2 msec
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    // Flatten summed accumulation downwards uniformly yielding discrete average
    return sum / LDR_SAMPLES;
}

/**
 * @brief Public getter for the filtered LDR value.
 * 
 * Leverages an Exponential Moving Average (EMA) on top of an averaged reading to 
 * provide a robust and non-jittering sensor readout.
 * 
 * @return int The filtered LDR value (0-4095).
 */
int bsp_ldr_read(void) 
{
    // Harvest uniform array average sample point over interval
    int avg = ldr_read_avg();
    // Apply standard generic exponential moving average formulas
    ema_value = EMA_ALPHA * avg + (1.0f - EMA_ALPHA) * ema_value;
    // Round the internal calculation cache structure down and return
    return (int)ema_value;
}

/**
 * @brief Public getter for the LDR voltage in millivolts.
 * 
 * Reads the filtered LDR raw value and extrapolates the equivalent hardware voltage 
 * utilizing the ESP ADC calibration.
 * 
 * @return int Calibration-adjusted voltage reading in mV. Returns 0 if calibration failed or uninitialized.
 */
int bsp_ldr_read_mv(void)
{
    // Check and escape quickly if calibration lookup schema wasn't bound
    if (cali_handle == NULL) return 0;
    // Reserve empty memory footprint readying post-transformation variable cache
    int mv = 0;
    // Inject filtered output fetching correlated hardware values
    adc_cali_raw_to_voltage(cali_handle, bsp_ldr_read(), &mv);
    // Expose physical electrical parameter outwards smoothly
    return mv;
}