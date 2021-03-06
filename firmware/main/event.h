#ifndef _EVENT_H_
#define _EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

typedef enum {
    ET_NONE,
    ET_ENCODER,
    ET_KEYBOARD,
    ET_WIFI,
} event_type_t;

typedef struct
{
    event_type_t type;
    uint8_t data[CONFIG_EVENT_MAX_SIZE];
} event_t;

extern QueueHandle_t event_queue;

#define event_cast(T, e) ((T*)e.data)
#define event_ptr_cast(T, e) ((T*)e->data)

typedef int (*event_handler_t)(event_t *);

esp_err_t event_init();
esp_err_t event_subscribe(event_type_t et, event_handler_t handler);
esp_err_t event_unsubscribe(event_type_t et, event_handler_t handler);
esp_err_t event_loop_start();

#ifdef __cplusplus
}
#endif

#endif /* _EVENT_H_ */
