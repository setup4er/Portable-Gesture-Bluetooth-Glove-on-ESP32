// C++ INCLUDES

// HEADERS
#include "adc.h"

// ESP-IDF
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

// ================== НАСТРОЙКИ ==================
#define BAT_ADC_UNIT      ADC_UNIT_1
#define BAT_ADC_CHANNEL   ADC_CHANNEL_5     // GPIO 33 = ADC1_CH5
#define BAT_ADC_ATTEN     ADC_ATTEN_DB_12   // ~0..3.3 В
#define BAT_ADC_BITWIDTH  ADC_BITWIDTH_DEFAULT

#define DIVIDER_RATIO     2.0f              // (R1+R2)/R2, для 100k+100k = 2

// Пороги Li-Ion (напряжение на самой банке)
#define BAT_FULL_MV       4200              // 100%
#define BAT_EMPTY_MV      3000              // 0%

static const char *TAG = "ADC";

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t         s_cali_handle = NULL;
static bool                      s_calibrated = false;

// ================== INIT ==================
void adc_init(void)
{
    // 1. Создание unit ADC1
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = BAT_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &s_adc_handle));

    // 2. Настройка канала
    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = BAT_ADC_BITWIDTH,
        .atten    = BAT_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, BAT_ADC_CHANNEL, &chan_cfg));

    // 3. Калибровка LINE FITTING (для классического ESP32)
    adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id  = BAT_ADC_UNIT,
        .atten    = BAT_ADC_ATTEN,
        .bitwidth = BAT_ADC_BITWIDTH,
    };
    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_cfg, &s_cali_handle);
    if (ret == ESP_OK) {
        s_calibrated = true;
        ESP_LOGI(TAG, "ADC инициализирован, калибровка line fitting активна");
    } else {
        s_calibrated = false;
        ESP_LOGW(TAG, "ADC инициализирован без калибровки");
    }
}

// ================== GET VOLTAGE ==================
float adc_get_voltage(void)
{
    if (s_adc_handle == NULL) return 0.0f;

    // --- Усреднение для стабильности ---
    const int N = 16;
    int raw_sum = 0;
    for (int i = 0; i < N; i++) {
        int raw = 0;
        if (adc_oneshot_read(s_adc_handle, BAT_ADC_CHANNEL, &raw) == ESP_OK) {
            raw_sum += raw;
        }
    }
    int raw_avg = raw_sum / N;

    // --- Перевод в мВ ---
    int voltage_mv = 0;
    if (s_calibrated) {
        adc_cali_raw_to_voltage(s_cali_handle, raw_avg, &voltage_mv);
    } else {
        voltage_mv = (raw_avg * 3300) / 4095;
    }

    // --- Напряжение на батарее (с учётом делителя) ---
    float battery_v = (voltage_mv / 1000.0f) * DIVIDER_RATIO;
    return battery_v;
}

// ================== GET PERCENT ==================
int adc_get_percent(void)
{
    float v = adc_get_voltage();
    int mv = (int)(v * 1000.0f);

    if (mv >= BAT_FULL_MV)  return 100;
    if (mv <= BAT_EMPTY_MV) return 0;

    int percent = (mv - BAT_EMPTY_MV) * 100 / (BAT_FULL_MV - BAT_EMPTY_MV);
    return percent;
}