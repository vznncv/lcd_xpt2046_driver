#include "lcd_xpt2046_driver.h"

int lcd_xpt2046_init_clear(lcd_xpt2046_driver_t *driver)
{
    driver->user_data = NULL;
    driver->communication_cb = NULL;
    driver->_is_initialized = 0;
    driver->lcd_width = -1;
    driver->lcd_height = -1;
    return 0;
}

#define XPT2046_CMD_CH_Z1 0xB0
#define XPT2046_CMD_CH_Z2 0xC0
#define XPT2046_CMD_CH_X 0x90
#define XPT2046_CMD_CH_Y 0xD0
#define XPT2046_CMD_CH_TEMP_0 0x80
#define XPT2046_CMD_CH_TEMP_1 0xF0

#define XPT2046_CMD_POWER_DOWN 0x00
#define XPT2046_CMD_POWER_UP 0x01

#define XPT2046_CONV_VAL(byte_0, byte_1) (byte_0 << 5 | byte_1 >> 3)

int lcd_xpt2046_init(lcd_xpt2046_driver_t *driver)
{
    int err = 0;

    if (driver->_is_initialized) {
        return -2;
    }

    if (driver->communication_cb == NULL) {
        return -1;
    }
    if (driver->lcd_width <= 0) {
        return -1;
    }
    if (driver->lcd_height <= 0) {
        return -1;
    }

    // try to measure z1 and z2 to check interface
    uint8_t in_buf[5] = { XPT2046_CMD_CH_Z1 | XPT2046_CMD_POWER_UP, 0x00, XPT2046_CMD_CH_Z2 | XPT2046_CMD_POWER_DOWN, 0x00, 0x00 };
    uint8_t out_buf[5] = { 0x00 };
    err = driver->communication_cb(driver->user_data, in_buf, out_buf, 5);
    if (err) {
        return err;
    }
    uint16_t z1 = XPT2046_CONV_VAL(out_buf[1], out_buf[2]);
    uint16_t z2 = XPT2046_CONV_VAL(out_buf[3], out_buf[4]);
    if (z1 == z2) {
        // something goes wrong
        return -1;
    }

    driver->_is_initialized = 1;
    return 0;
}

#define DELTA_Z_THRESHOLD 3800

static int16_t besttwoavg(int16_t v1, int16_t v2, int16_t v3)
{
    int16_t d12 = v1 > v2 ? v1 - v2 : v2 - v1;
    int16_t d13 = v1 > v3 ? v1 - v3 : v3 - v1;
    int16_t d23 = v2 > v3 ? v2 - v3 : v3 - v2;

    int16_t res;
    if (d12 <= d13 && d12 <= d23) {
        res = (v1 + v2) / 2;
    } else if (d13 <= d12 && d13 <= d23) {
        res = (v1 + v3) / 2;
    } else {
        res = (v2 + v3) / 2;
    }

    return res;
}

static int16_t scale_coord(int16_t x, float k)
{
    return (int16_t)((x - 0x800) * k) + 0x800;
}

#define X_COORD_SCALE 1.15f
#define Y_COORD_SCALE 1.15f

int lcd_xpt2046_read(lcd_xpt2046_driver_t *driver, lcd_xpt2046_point_t *point)
{
    int err;
    uint8_t in_buf[17] = {
        XPT2046_CMD_CH_Z1 | XPT2046_CMD_POWER_UP,
        0x00,
        XPT2046_CMD_CH_Z2 | XPT2046_CMD_POWER_UP,
        0x00,
        XPT2046_CMD_CH_X | XPT2046_CMD_POWER_UP,
        0x00,
        XPT2046_CMD_CH_Y | XPT2046_CMD_POWER_UP,
        0x00,
        XPT2046_CMD_CH_X | XPT2046_CMD_POWER_UP,
        0x00,
        XPT2046_CMD_CH_Y | XPT2046_CMD_POWER_UP,
        0x00,
        XPT2046_CMD_CH_X | XPT2046_CMD_POWER_UP,
        0x00,
        XPT2046_CMD_CH_Y | XPT2046_CMD_POWER_DOWN,
        0x00,
        0x00
    };
    uint8_t out_buf[17] = { 0 };
    err = driver->communication_cb(driver->user_data, in_buf, out_buf, 16);
    if (err) {
        return err;
    }

    int16_t z1 = XPT2046_CONV_VAL(out_buf[1], out_buf[2]);
    int16_t z2 = XPT2046_CONV_VAL(out_buf[3], out_buf[4]);

    int16_t dz = z2 - z1;

    if (dz < DELTA_Z_THRESHOLD) {
        // touch has been detected
        int y = besttwoavg(XPT2046_CONV_VAL(out_buf[5], out_buf[6]), XPT2046_CONV_VAL(out_buf[9], out_buf[10]), XPT2046_CONV_VAL(out_buf[13], out_buf[14]));
        int x = besttwoavg(XPT2046_CONV_VAL(out_buf[7], out_buf[8]), XPT2046_CONV_VAL(out_buf[11], out_buf[12]), XPT2046_CONV_VAL(out_buf[15], out_buf[16]));

        y = scale_coord(y, Y_COORD_SCALE);
        x = scale_coord(x, X_COORD_SCALE);

        point->x = x * driver->lcd_width / 0x1000;
        point->y = y * driver->lcd_height / 0x1000;
        point->z = 0xFFF - dz;

    } else {
        // there is no touch
        point->z = -1;
        point->x = -1;
        point->y = -1;
    }

    return 0;
}
