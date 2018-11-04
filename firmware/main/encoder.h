/**
 * This file is part of WAMP firmware
 *
 * Copyright (C) 2018 Ruslan V. Uss <unclerus@gmail.com>
 * MIT Licensed as described in the file LICENSE
 */
#ifndef _HW_ENCODER_H_
#define _HW_ENCODER_H_

#include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RE_BTN_RELEASED = 0,
    RE_BTN_PRESSED = 1,
    RE_BTN_LONG_PRESSED = 2
} rotary_encoder_btn_state_t;

typedef struct
{
    gpio_num_t pin_a, pin_b, pin_btn;
    int32_t value, step, min, max;
    uint8_t code, store;
    size_t index;
    uint64_t btn_pressed_time_us;
    rotary_encoder_btn_state_t btn_state;
} rotary_encoder_t;

typedef enum {
    RE_ET_CHANGED = 0,
    RE_ET_BTN_RELEASED,
    RE_ET_BTN_PRESSED,
    RE_ET_BTN_LONG_PRESSED
} rotary_encoder_event_type_t;

typedef struct
{
    rotary_encoder_event_type_t type;
    rotary_encoder_t *sender;
    int32_t diff;
} rotary_encoder_event_t;

esp_err_t rotary_encoder_init();
esp_err_t rotary_encoder_add(rotary_encoder_t *re);
esp_err_t rotary_encoder_remove(rotary_encoder_t *re);

#ifdef __cplusplus
}
#endif

#endif /* _HW_ENCODER_H_ */
