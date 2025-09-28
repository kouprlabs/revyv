#include "connector.h"
#include <memory>
#include <revyv/revyv.h>

using namespace revyv;

void* revyv_context_create()
{
    auto* connector = new Connector();
    return connector;
}

void revyv_context_destroy(void* ctx)
{
    auto* connector = (Connector*)ctx;
    delete connector;
}

uint32_t revyv_window_create(void* ctx, unsigned char* data, uint64_t data_size, double x, double y, double width, double height, uint8_t raster_type)
{
    auto* connector = (Connector*)ctx;
    return connector->window_create(data, data_size, x, y, width, height,
        (WindowRasterType)raster_type);
}

void revyv_window_update(void* ctx, uint32_t window_id, unsigned char* data, uint64_t data_size, double x, double y, double width, double height)
{
    auto* connector = (Connector*)ctx;
    connector->window_update_pixels(window_id, data, data_size, x, y, width, height);
}

void revyv_window_resize(void* ctx, uint32_t window_id, unsigned char* data, uint64_t data_size, double width, double height)
{
    auto* connector = (Connector*)ctx;
    connector->window_resize(window_id, data, data_size, width, height);
}

void revyv_window_change_visibility(void* ctx, uint32_t window_id, bool visible)
{
    auto* connector = (Connector*)ctx;
    connector->window_change_visiblity(window_id, visible);
}

void revyv_window_bring_to_front(void* ctx, uint32_t window_id)
{
    auto* connector = (Connector*)ctx;
    connector->window_bring_to_front(window_id);
}

void revyv_window_move(void* ctx, uint32_t window_id, double x, double y)
{
    auto* connector = (Connector*)ctx;
    connector->window_move(window_id, x, y);
}

void revyv_window_destroy(void* ctx, uint32_t window_id)
{
    auto* connector = (Connector*)ctx;
    connector->window_destroy(window_id);
}

RevyvEvent revyv_event_wait(void* ctx)
{
    auto* connector = (Connector*)ctx;

    RevyvEvent result {};

    std::shared_ptr<Event> event = connector->event_wait();

    result.type = event->get_type();

    if (event->get_type() == EventTypeMouseButton) {
        auto mouse_button_event = std::dynamic_pointer_cast<MouseButtonEvent>(event);
        result.mouse_event.button_state = mouse_button_event->get_state();
        result.mouse_event.button = mouse_button_event->get_button();
        result.mouse_event.clicks = mouse_button_event->get_clicks();
        result.mouse_event.x = mouse_button_event->get_window_x();
        result.mouse_event.y = mouse_button_event->get_window_y();
        result.mouse_event.abs_x = mouse_button_event->get_x();
        result.mouse_event.abs_y = mouse_button_event->get_y();
    } else if (event->get_type() == EventTypeMouseMove) {
        auto mouse_move_event = std::dynamic_pointer_cast<MouseMoveEvent>(event);
        result.mouse_event.x = mouse_move_event->get_window_x();
        result.mouse_event.y = mouse_move_event->get_window_y();
        result.mouse_event.abs_x = mouse_move_event->get_x();
        result.mouse_event.abs_y = mouse_move_event->get_y();
    } else if (event->get_type() == EventTypeMouseScroll) {
        auto mouse_scroll_event = std::dynamic_pointer_cast<MouseScrollEvent>(event);
        result.mouse_event.x = mouse_scroll_event->get_window_x();
        result.mouse_event.y = mouse_scroll_event->get_window_y();
        result.mouse_event.abs_x = mouse_scroll_event->get_x();
        result.mouse_event.abs_y = mouse_scroll_event->get_y();
        result.mouse_event.scroll_x = mouse_scroll_event->get_scroll_x();
        result.mouse_event.scroll_y = mouse_scroll_event->get_scroll_y();
    } else if (event->get_type() == EventTypeKey) {
        auto key_event = std::dynamic_pointer_cast<KeyEvent>(event);
        result.key_event.keycode = key_event->get_keycode();
        result.key_event.keymod = key_event->get_keymod();
        result.key_event.scancode = key_event->get_scancode();
        result.key_event.repeat = key_event->get_repeat();
        result.key_event.state = key_event->get_state();
    } else if (event->get_type() == EventTypeText) {
        auto text_event = std::dynamic_pointer_cast<TextEvent>(event);
        result.text_event.text = new char[text_event->get_text().size() + 1];
        strcpy(result.text_event.text, text_event->get_text().c_str());
        result.text_event.text_size = text_event->get_text().size();
    }

    return result;
}
