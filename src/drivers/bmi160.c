#include "bmi160.h"
#include "oled.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>
#include <stdint.h>

static const char *TAG = "BMI160";

#define BMI160_ADDR_1          0x68
#define BMI160_ADDR_2          0x69

#define BMI160_CHIP_ID         0x00
#define BMI160_GYR_X_L         0x0C
#define BMI160_GYR_CONF        0x42
#define BMI160_GYR_RANGE       0x43
#define BMI160_CMD             0x7E

#define BMI160_SOFT_RESET      0xB6
#define BMI160_GYR_NORMAL      0x15

/* 200 Hz gyro, normal bandwidth */
#define BMI160_GYR_CONF_200HZ  0x29

/* +/-500 dps */
#define BMI160_GYR_RANGE_500   0x02

/* +/-500 dps = 65.6 LSB/dps */
#define GYRO_SENSITIVITY       65.6f

#define MOUSE_SENSITIVITY      0.35f
#define MOUSE_DEADZONE         1.0f

/* Two-stage low-pass filter */
#define FILTER_ALPHA_1         0.20f
#define FILTER_ALPHA_2         0.35f

#define CALIBRATION_SAMPLES    200

static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t bmi160_dev = NULL;

static uint8_t bmi160_address = 0;
static bool bmi160_ready = false;

static float gyro_offset_x = 0.0f;
static float gyro_offset_y = 0.0f;
static float gyro_offset_z = 0.0f;

static float filtered_x = 0.0f;
static float filtered_y = 0.0f;

static float smooth_x = 0.0f;
static float smooth_y = 0.0f;

static float mouse_accum_x = 0.0f;
static float mouse_accum_y = 0.0f;


static esp_err_t bmi160_read(uint8_t reg, uint8_t *data, size_t len)
{
    if (bmi160_dev == NULL)
        return ESP_ERR_INVALID_STATE;

    return i2c_master_transmit_receive(
        bmi160_dev,
        &reg,
        1,
        data,
        len,
        100
    );
}


static esp_err_t bmi160_write(uint8_t reg, uint8_t value)
{
    if (bmi160_dev == NULL)
        return ESP_ERR_INVALID_STATE;

    uint8_t data[2] = {reg, value};

    return i2c_master_transmit(
        bmi160_dev,
        data,
        sizeof(data),
        100
    );
}


static bool bmi160_check_address(uint8_t address)
{
    i2c_master_dev_handle_t test_dev = NULL;

    i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = 400000,
    };

    if (i2c_master_bus_add_device(i2c_bus, &config, &test_dev) != ESP_OK)
        return false;

    uint8_t reg = BMI160_CHIP_ID;
    uint8_t chip_id = 0;

    esp_err_t err = i2c_master_transmit_receive(
        test_dev,
        &reg,
        1,
        &chip_id,
        1,
        100
    );

    i2c_master_bus_rm_device(test_dev);

    if (err != ESP_OK)
        return false;

    ESP_LOGI(
        TAG,
        "Address 0x%02X, CHIP_ID=0x%02X",
        address,
        chip_id
    );

    return chip_id == 0xD1;
}


static bool bmi160_read_gyro_raw(
    int16_t *gx,
    int16_t *gy,
    int16_t *gz
)
{
    uint8_t data[6];

    if (bmi160_read(BMI160_GYR_X_L, data, sizeof(data)) != ESP_OK)
        return false;

    *gx = (int16_t)(((uint16_t)data[1] << 8) | data[0]);
    *gy = (int16_t)(((uint16_t)data[3] << 8) | data[2]);
    *gz = (int16_t)(((uint16_t)data[5] << 8) | data[4]);

    return true;
}


void bmi160_calibrate(void)
{
    if (!bmi160_ready)
        return;

    ESP_LOGI(TAG, "Keep glove still. Calibration...");

    float sum_x = 0.0f;
    float sum_y = 0.0f;
    float sum_z = 0.0f;

    int valid_samples = 0;

    for (int i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        int16_t raw_x;
        int16_t raw_y;
        int16_t raw_z;

        if (bmi160_read_gyro_raw(&raw_x, &raw_y, &raw_z))
        {
            sum_x += raw_x;
            sum_y += raw_y;
            sum_z += raw_z;
            valid_samples++;
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }

    if (valid_samples == 0)
    {
        ESP_LOGW(TAG, "Calibration failed");
        return;
    }

    gyro_offset_x = sum_x / valid_samples;
    gyro_offset_y = sum_y / valid_samples;
    gyro_offset_z = sum_z / valid_samples;

    filtered_x = 0.0f;
    filtered_y = 0.0f;
    smooth_x = 0.0f;
    smooth_y = 0.0f;
    mouse_accum_x = 0.0f;
    mouse_accum_y = 0.0f;

    ESP_LOGI(
        TAG,
        "Offsets: X=%.1f Y=%.1f Z=%.1f",
        gyro_offset_x,
        gyro_offset_y,
        gyro_offset_z
    );
}


void bmi160_init(void)
{
    i2c_bus = oled_get_i2c_bus();

    if (i2c_bus == NULL)
    {
        ESP_LOGE(TAG, "OLED I2C bus is NULL");
        return;
    }

    if (bmi160_check_address(BMI160_ADDR_1))
        bmi160_address = BMI160_ADDR_1;
    else if (bmi160_check_address(BMI160_ADDR_2))
        bmi160_address = BMI160_ADDR_2;
    else
    {
        ESP_LOGE(TAG, "BMI160 not found");
        return;
    }

    i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = bmi160_address,
        .scl_speed_hz = 400000,
    };

    esp_err_t err = i2c_master_bus_add_device(
        i2c_bus,
        &config,
        &bmi160_dev
    );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to add BMI160: %s",
            esp_err_to_name(err)
        );
        bmi160_dev = NULL;
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(10));

    if (bmi160_write(BMI160_CMD, BMI160_SOFT_RESET) != ESP_OK)
    {
        ESP_LOGE(TAG, "BMI160 soft reset failed");
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    if (bmi160_write(BMI160_CMD, BMI160_GYR_NORMAL) != ESP_OK)
    {
        ESP_LOGE(TAG, "BMI160 gyro start failed");
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(80));

    if (bmi160_write(BMI160_GYR_CONF, BMI160_GYR_CONF_200HZ) != ESP_OK)
    {
        ESP_LOGE(TAG, "BMI160 gyro config failed");
        return;
    }

    if (bmi160_write(BMI160_GYR_RANGE, BMI160_GYR_RANGE_500) != ESP_OK)
    {
        ESP_LOGE(TAG, "BMI160 gyro range failed");
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(20));

    bmi160_ready = true;

    ESP_LOGI(TAG, "BMI160 initialized");

    bmi160_calibrate();
}


bool bmi160_is_ready(void)
{
    return bmi160_ready;
}


bool bmi160_get_mouse_delta(int *dx, int *dy)
{
    if (dx == NULL || dy == NULL)
        return false;

    *dx = 0;
    *dy = 0;

    if (!bmi160_ready)
        return false;

    int16_t raw_x;
    int16_t raw_y;
    int16_t raw_z;

    if (!bmi160_read_gyro_raw(&raw_x, &raw_y, &raw_z))
        return false;

    float gx =
        ((float)raw_x - gyro_offset_x) /
        GYRO_SENSITIVITY;

    float gy =
        ((float)raw_y - gyro_offset_y) /
        GYRO_SENSITIVITY;

    if (fabsf(gx) < MOUSE_DEADZONE)
        gx = 0.0f;

    if (fabsf(gy) < MOUSE_DEADZONE)
        gy = 0.0f;

    filtered_x =
        filtered_x * (1.0f - FILTER_ALPHA_1) +
        gx * FILTER_ALPHA_1;

    filtered_y =
        filtered_y * (1.0f - FILTER_ALPHA_1) +
        gy * FILTER_ALPHA_1;

    smooth_x =
        smooth_x * (1.0f - FILTER_ALPHA_2) +
        filtered_x * FILTER_ALPHA_2;

    smooth_y =
        smooth_y * (1.0f - FILTER_ALPHA_2) +
        filtered_y * FILTER_ALPHA_2;

    /*
     * Horizontal axis is inverted so that turning
     * the hand left moves the cursor left.
     */
    float move_x = -smooth_y * MOUSE_SENSITIVITY;
    float move_y = -smooth_x * MOUSE_SENSITIVITY;

    mouse_accum_x += move_x;
    mouse_accum_y += move_y;

    int out_x = (int)mouse_accum_x;
    int out_y = (int)mouse_accum_y;

    mouse_accum_x -= out_x;
    mouse_accum_y -= out_y;

    *dx = out_x;
    *dy = out_y;

    return true;
}
