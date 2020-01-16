#ifndef LCD_XPT2046_DRIVER
#define LCD_XPT2046_DRIVER

#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Common XPT2046 LCD touchpad driver.
 *
 * Driver usage:
 *
 * 1. Create and clear driver:
 *
 * @code{.cpp}
 * lcd_xpt2046_driver_t touch_driver;
 * lcd_xpt2046_init_clear(&touch_driver);
 * @endcode
 *
 * 2. Attach callbacks and set screen resolution:
 *
 * @code{.cpp}
 * // Set custom data. If you don't need any data, assign NULL.
 * touch_driver.user_data = my_data_ptr;
 * // Set communication callback
 * touch_driver.communication_cb = my_communication_cb;
 * // Set screen resolution
 * touch_driver.lcd_width = 240;
 * touch_driver.lcd_height = 320;
 * @endcode
 *
 * 3. Initialize driver:
 *
 * @code{.cpp}
 * int res = lcd_xpt2046_init(&touch_driver);
 * if (!res) {
 *     // Driver initialization error.
 *     // Show error somehow and stop application
 *     // ...
 * }
 * @endcode
 *
 * 4. Use driver to get touch position.
 *
 * @code{.cpp}
 * lcd_xpt2046_point_t point;
 *
 * lcd_xpt2046_read(&touch_driver,&point);
 *
 * if (point.z >= 0) {
 *    // process touch
 *    ...
 * } else {
 *    // no touches are detected
 *    ...
 * }
 * ...
 * @endcode
 *
 */
typedef struct {
    /**
     * custom data.
     */
    void *user_data;
    /**
     * Touch sensor communication callback.
     *
     * @param lcd_data custom data
     * @param out_buf buffer with data to transfer
     * @param in_buf buffer with data to receive
     * @param size transer buffers length
     * @return zero on success, otherwise non-zero value
     */
    int (*communication_cb)(void *user_data, uint8_t *out_buf, uint8_t *in_buf, size_t size);

    /**
     * X display resolution.
     */
    int16_t lcd_width;
    /**
     * Y display resolution.
     */
    int16_t lcd_height;

    // private variables
    int _is_initialized;
} lcd_xpt2046_driver_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} lcd_xpt2046_point_t;

/**
 * Clear lcd_xpt2046_driver_t driver struct.
 *
 * @param driver
 * @return zero on success, otherwise non-zero value
 */
int lcd_xpt2046_init_clear(lcd_xpt2046_driver_t *driver);

/**
 * Initialize LCD touch controller.
 *
 * @param driver
 * @return zero on success, otherwise non-zero value
 */
int lcd_xpt2046_init(lcd_xpt2046_driver_t *driver);

/**
 * Read touchpad state.
 *
 * If touch is detected, then `x`, `y` fields of the point argument will be set to corresponding position and `z` to non-negative value.
 * If touch isn't detected, then `x`, `y` and `z` will be set to -1.
 *
 * @param driver
 * @param point
 * @return zero on success, otherwise non-zero value
 */
int lcd_xpt2046_read(lcd_xpt2046_driver_t *driver, lcd_xpt2046_point_t *point);

#ifdef __cplusplus
}
#endif

#endif
