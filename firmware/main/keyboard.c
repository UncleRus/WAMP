#include "keyboard.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <pcf8574.h>
#include <esp_log.h>
#include "event.h"
#include "common.h"

static const char *TAG = "KEYBOARD";

static i2c_dev_t expander;
static uint8_t old_val = 0xff; // FIXME

static TaskHandle_t read_task;

static void IRAM_ATTR isr_handler(void *arg)
{
    vTaskNotifyGiveFromISR(read_task, NULL);
}

static void read_port(void *arg)
{
    uint8_t val;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ESP_LOGV(TAG, "Reading port...");
        if (pcf8574_port_read(&expander, &val) != ESP_OK)
        {
            ESP_LOGE(TAG, "Could not read PCF8574");
            continue;
        }

        if (val != old_val)
        {
            ESP_LOGV(TAG, "Old port value: 0x%02x, new: 0x%02x", old_val, val);

            event_t e;
            e.type = ET_KEYBOARD;
            keyboard_event_t *ke = event_cast(keyboard_event_t, e);
            ke->state = val;
            ke->old_state = old_val;

            old_val = val;

            if (!xQueueSend(event_queue, &e, CONFIG_KEYBOARD_EVENT_TIMEOUT / portTICK_PERIOD_MS))
                ESP_LOGW(TAG, "Timeout occured while sending keyboard event");
        }
    }
}

esp_err_t keyboard_init()
{
    CHECK(pcf8574_init_desc(&expander, CONFIG_I2C_PORT, CONFIG_KEYBOARD_ADDR, CONFIG_I2C_SDA_GPIO, CONFIG_I2C_SCL_GPIO));
    if (xTaskCreate(read_port, TAG, CONFIG_KEYBOARD_TASK_STACK, NULL, CONFIG_KEYBOARD_TASK_PRIORITY, &read_task) != pdPASS)
    {
        ESP_LOGE(TAG, "Could not create task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "PCF8574 ready");

    CHECK(gpio_set_direction(CONFIG_KEYBOARD_INTR_GPIO, GPIO_MODE_INPUT));
    CHECK(gpio_set_pull_mode(CONFIG_KEYBOARD_INTR_GPIO, GPIO_PULLUP_ONLY));
    CHECK(gpio_set_intr_type(CONFIG_KEYBOARD_INTR_GPIO, GPIO_INTR_LOW_LEVEL));

    CHECK(gpio_install_isr_service(0));
    CHECK(gpio_isr_handler_add(CONFIG_KEYBOARD_INTR_GPIO, isr_handler, NULL));

    ESP_LOGI(TAG, "GPIO %d ready", CONFIG_KEYBOARD_INTR_GPIO);

    return ESP_OK;
}
