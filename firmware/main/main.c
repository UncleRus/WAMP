#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include "config.h"
#include "event.h"
#include "encoder.h"

static rotary_encoder_t re;

static void controls_init()
{
    ESP_ERROR_CHECK(rotary_encoder_init());

    // Setup rotary encoder
    memset(&re, 0, sizeof(rotary_encoder_t));
    re.pin_a = CONFIG_RE_A_GPIO;
    re.pin_b = CONFIG_RE_B_GPIO;
    re.pin_btn = CONFIG_RE_BTN_GPIO;
    re.min = 0;
    re.max = 0xfffff;
    re.value = 0;
    re.step = 1;
    ESP_ERROR_CHECK(rotary_encoder_add(&re));

    // Setup keyboard
}

static int encoder_handler(event_t *ev)
{
    rotary_encoder_event_t *e = event_ptr_cast(rotary_encoder_event_t, ev);
    printf("Got encoder event. type = %d, sender = 0x%08x, diff = %d\n", e->type, (uint32_t)e->sender, e->diff);
    return 0;
}

void app_main()
{
    ESP_ERROR_CHECK(event_init());
    controls_init();

    ESP_ERROR_CHECK(event_subscribe(ET_ENCODER, encoder_handler));

    ESP_ERROR_CHECK(event_loop_start());
}

