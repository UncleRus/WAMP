#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    KEY_DISPLAY = 0,
    KEY_MUTE,
    KEY_INPUT,
    KEY_PHONES,
    KEY_DEFEAT,
    KEY_SETTINGS,
    KEY_PLUS,
    KEY_MINUS
} keyboard_key_t;

typedef struct {
    uint8_t state, old_state;
} keyboard_event_t;

esp_err_t keyboard_init();

#ifdef __cplusplus
}
#endif

#endif /* _KEYBOARD_H_ */
