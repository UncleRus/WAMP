#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <i2cdev.h>
#include "config.h"
#include "event.h"
#include "encoder.h"
#include "keyboard.h"

static rotary_encoder_t re;

static void controls_init()
{
    ESP_ERROR_CHECK(i2cdev_init());

    // Setup keyboard
    ESP_ERROR_CHECK(keyboard_init());

    // Setup rotary encoder
    ESP_ERROR_CHECK(rotary_encoder_init());

    memset(&re, 0, sizeof(rotary_encoder_t));
    re.pin_a = CONFIG_RE_A_GPIO;
    re.pin_b = CONFIG_RE_B_GPIO;
    re.pin_btn = CONFIG_RE_BTN_GPIO;
    re.min = 0;
    re.max = 0xfffff;
    re.value = 0;
    re.step = 1;
    ESP_ERROR_CHECK(rotary_encoder_add(&re));
}

static int encoder_handler(event_t *ev)
{
    rotary_encoder_event_t *e = event_ptr_cast(rotary_encoder_event_t, ev);
    printf("Got encoder event. type = %d, sender = 0x%08x, diff = %d\n", e->type, (uint32_t)e->sender, e->diff);
    return 0;
}

static int keyboard_handler(event_t *ev)
{
    keyboard_event_t *e = event_ptr_cast(keyboard_event_t, ev);
    printf("Got keyboard event. Old state: 0x%02x, new state: 0x%02x\n", e->old_state, e->state);
    return 0;
}

void app_main()
{
    ESP_ERROR_CHECK(event_init());
    controls_init();

    ESP_ERROR_CHECK(event_subscribe(ET_ENCODER, encoder_handler));
    ESP_ERROR_CHECK(event_subscribe(ET_KEYBOARD, keyboard_handler));

    ESP_ERROR_CHECK(event_loop_start());
}

