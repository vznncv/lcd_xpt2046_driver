# XPT2046 LCD driver

The library represents minimal hardware independent driver for "XPT2046" LCD touch controller
for testing and education purposes.

## Basic usage

```
// Create and clear driver
lcd_xpt2046_driver_t touch_driver;
lcd_xpt2046_init_clear(&touch_driver);

// Set custom data. If you don't need any data, assign NULL.
touch_driver.user_data = my_data_ptr;
// Set communication callback
touch_driver.communication_cb = my_communication_cb;
// Set screen resolution
touch_driver.lcd_width = 240;
touch_driver.lcd_height = 320;

// Initialize driver
int res = lcd_xpt2046_init(&touch_driver);
if (!res) {
     // Driver initialization error.
     // Show error somehow and stop application
     // ...
}

// Use driver to get touch position.
lcd_xpt2046_point_t point;

lcd_xpt2046_read(&touch_driver,&point);
if (point.z >= 0) {
   // process touch
   ...
} else {
   // no touches are detected
   ...
}
```
