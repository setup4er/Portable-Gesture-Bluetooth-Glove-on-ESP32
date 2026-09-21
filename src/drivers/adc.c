#include "adc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

#define BAT_ADC_UNIT       ADC_UNIT_1
#define BAT_ADC_CHANNEL    ADC_CHANNEL_5       // GPIO33
#define BAT_ADC_ATTEN      ADC_ATTEN_DB_12
#define BAT_ADC_BITWIDTH   ADC_BITWIDTH_DEFAULT

// 10k + 10k divider:
// Battery+ -> 10k -> GPIO33 -> 10k -> GND
#define DIVIDER_RATIO      2.0f

#define BAT_FULL_MV        4200
#define BAT_EMPTY_MV       3000

#define ADC_SAMPLES         16

static const char *TAG = "BAT";

static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;

static bool calibration_enabled = false;


// --------------------------------------------------
// ADC INIT
// --------------------------------------------------

void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = BAT_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&init_config, &adc_handle)
    );

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = BAT_ADC_BITWIDTH,
        .atten = BAT_ADC_ATTEN,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            BAT_ADC_CHANNEL,
            &config
        )
    );

    // Calibration
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = BAT_ADC_UNIT,
        .atten = BAT_ADC_ATTEN,
        .bitwidth = BAT_ADC_BITWIDTH,
    };

    esp_err_t ret =
        adc_cali_create_scheme_line_fitting(
            &cali_config,
            &adc_cali_handle
        );

    if (ret == ESP_OK) {
        calibration_enabled = true;
        ESP_LOGI(TAG, "ADC calibration enabled");
    } else {
        calibration_enabled = false;
        ESP_LOGW(TAG, "ADC calibration unavailable");
    }
}


// --------------------------------------------------
// READ BATTERY VOLTAGE
// --------------------------------------------------

float adc_get_voltage(void)
{
    if (adc_handle == NULL) {
        return 0.0f;
    }

    int raw = 0;
    int raw_sum = 0;
    int successful_samples = 0;

    for (int i = 0; i < ADC_SAMPLES; i++) {

        esp_err_t ret =
            adc_oneshot_read(
                adc_handle,
                BAT_ADC_CHANNEL,
                &raw
            );

        if (ret == ESP_OK) {
            raw_sum += raw;
            successful_samples++;
        }
    }

    if (successful_samples == 0) {
        ESP_LOGW(TAG, "ADC read failed");
        return 0.0f;
    }

    int raw_avg = raw_sum / successful_samples;

    int adc_mv = 0;

    if (calibration_enabled) {

        esp_err_t ret =
            adc_cali_raw_to_voltage(
                adc_cali_handle,
                raw_avg,
                &adc_mv
            );

        if (ret != ESP_OK) {
            adc_mv =
                (raw_avg * 3300) / 4095;
        }

    } else {

        adc_mv =
            (raw_avg * 3300) / 4095;
    }

    // ADC sees half of battery voltage
    float battery_mv =
        adc_mv * DIVIDER_RATIO;

    float battery_voltage =
        battery_mv / 1000.0f;

    ESP_LOGI(
        TAG,
        "RAW=%d | ADC=%d mV | BAT=%.3f V",
        raw_avg,
        adc_mv,
        battery_voltage
    );

    return battery_voltage;
}


// --------------------------------------------------
// BATTERY PERCENT
// --------------------------------------------------

int adc_voltage_to_percent(float voltage)
{
    int mv = (int)(voltage * 1000.0f);

    int percent =
        (mv - BAT_EMPTY_MV) * 100 /
        (BAT_FULL_MV - BAT_EMPTY_MV);

    if (percent < 0)
        percent = 0;

    if (percent > 100)
        percent = 100;

    return percent;
}


int adc_get_percent(void)
{
    float voltage = adc_get_voltage();

    return adc_voltage_to_percent(voltage);
}