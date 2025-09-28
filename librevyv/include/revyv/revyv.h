#ifndef REVYV_H
#define REVYV_H

#define EXPORT __attribute__((visibility("default")))

#include <cstdint>
#include <cstdio>
#include <revyv/keycodes.h>

typedef enum : uint8_t {
    RevyvWindowRasterRGBA = 1,
    RevyvWindowRasterARGB = 2,
} RevyvWindowRasterType;

typedef enum : uint8_t {
    RevyvEventTypeUndefined = 0,
    RevyvEventTypeMouseMove = 1,
    RevyvEventTypeMouseButton = 2,
    RevyvEventTypeMouseScroll = 3,
    RevyvEventTypeKey = 4,
    RevyvEventTypeText = 5,
    RevyvEventTypeQuit = 6,
} RevyvEventType;

typedef enum {
    RevyvMouseButtonTypeUndefined = 0,
    RevyvMouseButtonTypeRight = 1,
    RevyvMouseButtonTypeLeft = 2,
    RevyvMouseButtonTypeMiddle = 3,
} RevyvMouseButtonType;

typedef enum {
    RevyvMouseButtonStateUndefined = 0,
    RevyvMouseButtonStatePressed = 1,
    RevyvMouseButtonStateReleased = 2,
} RevyvMouseButtonState;

typedef enum {
    RevyvKeyStateUndefined = 0,
    RevyvKeyStatePressed = 1,
    RevyvKeyStateReleased = 2,
} RevyvKeyState;

extern "C" {

typedef struct
{
    uint8_t button_state;
    uint8_t button;
    uint8_t clicks;
    double x;
    double y;
    double abs_x;
    double abs_y;
    int32_t scroll_x;
    int32_t scroll_y;
} RevyvMouseEvent;

typedef struct
{
    int32_t scancode;
    int32_t keycode;
    int32_t keymod;
    int8_t state;
    bool repeat;
} RevyvKeyEvent;

typedef struct
{
    char* text;
    size_t text_size;
} RevyvTextEvent;

typedef struct
{
    uint32_t window_id;
    uint8_t type;
    RevyvKeyEvent key_event;
    RevyvTextEvent text_event;
    RevyvMouseEvent mouse_event;
} RevyvEvent;

EXPORT void* revyv_context_create();

EXPORT void revyv_context_destroy(void* context);

EXPORT uint32_t revyv_window_create(void* context, unsigned char* data, size_t data_size, double x, double y, double width, double height, uint8_t raster_type);

EXPORT void revyv_window_update(void* context, uint32_t window_id, unsigned char* data, size_t data_size, double x, double y, double width, double height);

EXPORT void revyv_window_resize(void* context, uint32_t window_id, void* data, uint64_t data_size, double width, double height);

EXPORT void revyv_window_change_visibility(void* context, uint32_t window_id, bool visible);

EXPORT void revyv_window_bring_to_front(void* context, uint32_t window_id);

EXPORT void revyv_window_move(void* context, uint32_t window_id, double x, double y);

EXPORT void revyv_window_destroy(void* context, uint32_t window_id);

EXPORT RevyvEvent revyv_event_wait(void* context);
}

#endif
