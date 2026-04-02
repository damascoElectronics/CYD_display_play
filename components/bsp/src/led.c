/**
 * @file led.c
 * @brief RGB LED control and effect implementations.
 */
#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"     
#include "bsp_pinout.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Heartbeat curve representing PWM values (0-255).
 * 
 * Produce highly specific constant memory array defining visual pulsation profile
 */
static const uint8_t heartbeat_curve[] = 
{ 
    0,  10,  30,  60, 100, 150, 200, 240, 255, 220,  // Synthesize smooth ascent terminating into a high plateau
  170, 110,  60,  80, 120, 170, 210, 240, 220, 180,  // Synthesize mild descent followed instantly by minor secondary peak
  130,  80,  40,  15,   5,   0,   0,   0,   0,   0,  // Rapid taper entirely zeroing out
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // Define formal delay window keeping device visually off indefinitely
};

/** @brief Total number of steps in the heartbeat curve array 
 * 
 * Standard dynamic sizeof methodology measuring total array frame length
*/
#define HEARTBEAT_STEPS  (sizeof(heartbeat_curve) / sizeof(heartbeat_curve[0]))

/**
 * @brief Initializes the RGB LED GPIO and LEDC timer/channel configurations.
 * 
 * Configures the GPIO direction and sets up the LEDC timer and channels 
 * for PWM control of the RGB LED. Also spawns the led_effect_task.
 *
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG on failure.
 */
esp_err_t init_RGB_led(void)
{
    // Request ESP config layer clear any leftover RED pin reservations
    if (gpio_reset_pin(BSP_LED_R) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream abandoning initialization phase, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Request ESP config layer clear any leftover GREEN pin reservations
    if (gpio_reset_pin(BSP_LED_G) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream abandoning initialization phase, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Request ESP config layer clear any leftover BLUE pin reservations
    if (gpio_reset_pin(BSP_LED_B) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream abandoning initialization phase, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }

    // Request direct output mode specifically allocated referencing RED pin
    if (gpio_set_direction(BSP_LED_R, GPIO_MODE_OUTPUT) == ESP_ERR_INVALID_ARG) 
    {
        // Escalate fault upstream abandoning initialization phase, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Request direct output mode specifically allocated referencing GREEN pin
    if (gpio_set_direction(BSP_LED_G, GPIO_MODE_OUTPUT) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream abandoning initialization phase, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Request direct output mode specifically allocated referencing BLUE pin
    if (gpio_set_direction(BSP_LED_B, GPIO_MODE_OUTPUT) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream abandoning initialization phase, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    
    // Build single timer configuration bundle meant broadly supporting all RGB phases
    ledc_timer_config_t timer = {    
        .speed_mode      = LEDC_LOW_SPEED_MODE, // Assure generic low-speed stability targeting simpler peripherals
        .timer_num       = BSP_LED_LEDC_TIMER,  // Map specifically to predefined system free timer resource
        .duty_resolution = BSP_LED_LEDC_RES,    // Ensure resolution bit depths appropriately encapsulate expected logic 1-255 spectrums
        .freq_hz         = BSP_LED_LEDC_FREQ,   // Modulate PWM signal accurately conforming preset definitions
        .clk_cfg         = LEDC_AUTO_CLK,       // Ensure timer clock synchronization picks default lowest footprint resource
    };
    // Apply constructed timer profile checking hardware bounds automatically
    ESP_ERROR_CHECK(ledc_timer_config(&timer)); 
    // Give internal hardware states 10 milliseconds to physically latch logic levels
    vTaskDelay(pdMS_TO_TICKS(10));
    // Collate independent physical LED pins uniformly inside standard sequence
    gpio_num_t pins[3]         = {BSP_LED_R, BSP_LED_G, BSP_LED_B};
    // Collate independent PWM channels mirroring parallel hardware configuration
    ledc_channel_t channels[3] = {BSP_LED_CH_R, BSP_LED_CH_G, BSP_LED_CH_B};
    // Initialize iterator cascading sequentially configuring individual elements automatically
    for (int i = 0; i < 3; i++) {    
        // Generate explicit channel profile explicitly overwriting properties per iteration step
        ledc_channel_config_t ch = { 
            .gpio_num   = pins[i],   // Force respective specific parallel physical wire bindings iteratively
            .speed_mode = LEDC_LOW_SPEED_MODE, // Synchronize timing domain compatibility identical with prior timer build
            .channel    = channels[i], // Register independent hardware generation channels properly splitting execution streams
            .timer_sel  = BSP_LED_LEDC_TIMER, // Tie all streams commonly integrating into singular timing source heartbeat
            .duty       = 255,   // Assure startup properties boot logically turned completely transparently high
            .hpoint     = 0,     // Configure specific clock start point logically standardizing to zero
            .flags.output_invert = 0, // Inversion parameters explicitly mapped disabled defaulting normal Boolean behaviors
        };
        // Force iteration explicitly checking against underlying system boundaries assuring total safety
        ESP_ERROR_CHECK(ledc_channel_config(&ch));
    }
    // Launch parallel FreeRTOS agent running dynamic fading
    xTaskCreate(led_effect_task, "led_effect", 2048, NULL, 0, NULL);
    // if all goes well, return ESP_OK
    return ESP_OK;
}

/**
 * @brief Turns off the RGB LED by setting the respective GPIO levels to High (off state).
 * 
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG upon failure.
 */
esp_err_t turn_off_RGB_led(void)
{
    
    // Dispatch standardized zero state parameters functionally blanking visual outputs completely
    if (bsp_led_set(0, 0, 0) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // if all goes well, return ESP_OK
    return ESP_OK;
}

/**
 * @brief Turns on the RGB LED by setting the respective GPIO levels to Low (on state).
 * 
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG upon failure.
 */
esp_err_t turn_on_RGB_led(void)
{
    // Dispatch standardized maximum parameters logically blinding outputs
    if (bsp_led_set(255, 255, 255) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // if all goes well, return ESP_OK
    return ESP_OK;
}

/**
 * @brief Controls a single color channel of the RGB LED.
 * 
 * @param gpio_num The GPIO number of the color channel.
 * @param level The desired logical level (true for ON, false for OFF).
 * @return esp_err_t ESP_OK on success, or ESP_ERR_INVALID_ARG upon failure.
 */
esp_err_t control_single_color_RGB_led(gpio_num_t gpio_num, bool level)
{
    // Pass logically inverted boolean state mapping standard inputs seamlessly mapping towards hardware expectation layers natively
    return gpio_set_level(gpio_num, !level); 
}

/**
 * @brief Sets the duty cycle for the RGB LED via LEDC.
 * 
 * @param r Red duty cycle value (0-255).
 * @param g Green duty cycle value (0-255).
 * @param b Blue duty cycle value (0-255).
 */
esp_err_t bsp_led_set(uint8_t r, uint8_t g, uint8_t b) 
{
    // Establish modified Red parameters converting 255-oriented mapping translating native polarity configurations inversely
    if (ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_R, 255 - r) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Establish modified Green parameters converting 255-oriented mapping translating native polarity configurations inversely 
    if (ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_G, 255 - g) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Establish modified Blue parameters converting 255-oriented mapping translating native polarity configurations inversely
    if (ledc_set_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_B, 255 - b) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }   
    // Dispatch physical hardware push targeting native runtime channel specifically committing Red parameters accurately
    if (ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_R) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // Dispatch physical hardware push targeting native runtime channel specifically committing Green parameters accurately
    if (ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_G) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
        // Dispatch physical hardware push targeting native runtime channel specifically committing Blue parameters accurately
    if (ledc_update_duty(LEDC_LOW_SPEED_MODE, BSP_LED_CH_B) == ESP_ERR_INVALID_ARG)
    {
        // Escalate fault upstream, return ESP_ERR_INVALID_ARG
        return ESP_ERR_INVALID_ARG;
    }
    // if all goes well, return ESP_OK
    return ESP_OK;
}

/**
 * @brief Call this function periodically (e.g., every 20ms) to 
 *        obtain the next PWM value of the heartbeat effect.
 *
 * @return uint8_t  PWM value [0-255]
 */
uint8_t heartbeat_tick(void)
{
    // Establish persistent internal memory slot ensuring index frame maintains timeline progression safely
    static uint16_t step = 0;
    // Extract specific literal numerical value corresponding accurately tracing memory mapped sequence internally
    uint8_t pwm_val = heartbeat_curve[step]; 
    // Increment persistent timeline step forcefully advancing index pointer sequentially ahead
    step++;
    // Proactively ensure forward progression never breaches predefined array boundaries overflowing into invalid space memory explicitly
    if (step >= HEARTBEAT_STEPS)
    {
        // Execute wrap-around protocol defaulting timeline parameters effectively repeating effect cycle infinitely
        step = 0;                    
    }
    return pwm_val;
}

/**
 * @brief FreeRTOS task handling a heartbeat effect for the RGB LED.
 * 
 * Cycles the heartbeat pattern through the Red, Green, and Blue channels.
 * 
 * Primary multithreading process defining continuous isolated cyclic visual performance gracefully
 * 
 * @param pvParameters Pointer to task parameters (not used).
 */
void led_effect_task(void *pvParameters) {
    uint8_t hue = 0;                 // Create isolated localized storage strictly interpreting dynamically produced chronological values effectively
    // Command operating system standard executing infinite task bounds loop unconditionally securely
    while (1) 
    {
        // Request newly fetched visual value logically polling isolated helper extracting state values consistently
        hue = heartbeat_tick();
        // Dump identical extracted state across distinct colored boundaries synthesizing monochrome visual dynamics evenly
        bsp_led_set(hue, hue, hue);
        // Delay entirely preventing total CPU utilization effectively managing specific 30ms cyclic task progression logically
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}