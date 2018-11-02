#include "event.h"
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "common.h"

QueueHandle_t event_queue = NULL;

static const char *TAG = "EVENT";
static bool started = false;

typedef struct
{
    event_type_t et;
    event_handler_t handler;
} subscriber_t;

static subscriber_t subs[CONFIG_EVENT_MAX_LISTENERS];
static SemaphoreHandle_t mutex;

#define TIMEOUT 10 // 100ms

esp_err_t event_init()
{
    event_queue = xQueueCreate(CONFIG_EVENT_QUEUE_SIZE, CONFIG_EVENT_MAX_SIZE);
    if (!event_queue)
    {
        ESP_LOGE(TAG, "Could not create queue");
        return ESP_ERR_NO_MEM;
    }

    memset(subs, 0, sizeof(subs));
    mutex = xSemaphoreCreateMutex();
    if (!mutex)
    {
        ESP_LOGE(TAG, "Could not create mutex");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Event queue ready");
    return ESP_OK;
}

esp_err_t event_subscribe(event_type_t et, event_handler_t handler)
{
    CHECK_ARG(handler);
    CHECK_ARG(et != ET_NONE);

    if (!xSemaphoreTake(mutex, TIMEOUT))
    {
        ESP_LOGE(TAG, "Could not subscribe 0x%08x to %d: Timeout", (uint32_t)handler, et);
        return ESP_ERR_TIMEOUT;
    }

    for (size_t i = 0; i < CONFIG_EVENT_MAX_LISTENERS; i ++)
        if (subs[i].et == ET_NONE)
        {
            subs[i].et = et;
            subs[i].handler = handler;
            xSemaphoreGive(mutex);

            ESP_LOGD(TAG, "0x%08x subscribed to %d", (uint32_t)handler, et);
            return ESP_OK;
        }

    xSemaphoreGive(mutex);

    ESP_LOGE(TAG, "Could not subscribe 0x%08x to %d: Out of memory", (uint32_t)handler, et);
    return ESP_ERR_NO_MEM;
}

esp_err_t event_unsubscribe(event_type_t et, event_handler_t handler)
{
    if (!xSemaphoreTake(mutex, TIMEOUT))
    {
        ESP_LOGE(TAG, "Could not unsubscribe 0x%08x from %d: Timeout", (uint32_t)handler, et);
        return ESP_ERR_TIMEOUT;
    }

    for (size_t i = 0; i < CONFIG_EVENT_MAX_LISTENERS; i ++)
    {
        if (subs[i].et == ET_NONE)
            continue;

        if ((et == ET_NONE && subs[i].handler == handler)
                || (subs[i].et == et && subs[i].handler == handler)
                || (subs[i].et == et && !handler))
        {
            ESP_LOGD(TAG, "0x%08x unsubscribed from %d", (uint32_t)subs[i].handler, subs[i].et);
            memset(&subs[i], 0, sizeof(subscriber_t));
        }
    }

    xSemaphoreGive(mutex);

    return ESP_OK;
}

static void event_loop(void *arg)
{
    while (1)
    {
        event_t e;

        xQueueReceive(event_queue, &e, portMAX_DELAY);

        if (e.type != ET_NONE)
        {
            ESP_LOGV(TAG, "Processing event %d", e.type);
            for (size_t i = 0; i < CONFIG_EVENT_MAX_LISTENERS; i ++)
            {
                if (subs[i].et == e.type && subs[i].handler)
                    if (subs[i].handler(&e))
                        break;
            }
        }
    }
}

esp_err_t event_loop_start()
{
    if (started)
    {
        ESP_LOGE(TAG, "Event loop already started");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Starting event loop...");
    if (xTaskCreatePinnedToCore(event_loop, TAG, CONFIG_EVENT_LOOP_STACK_SIZE, NULL, CONFIG_EVENT_LOOP_PRIORITY,
            NULL, CONFIG_EVENT_LOOP_CPU) != pdPASS)
    {
        ESP_LOGE(TAG, "Could not create event loop task");
        return ESP_ERR_NO_MEM;
    }

    started = true;

    return ESP_OK;
}
