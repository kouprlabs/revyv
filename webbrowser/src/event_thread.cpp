#include "event_thread.h"
#include "chromium_keycodes.h"
#include "include/cef_app.h"
#include "include/cef_render_handler.h"
#include <revyv/revyv.h>

void EventThread::event_thread(CefBrowser* browser, void* aslCtx)
{
    RevyvEvent event;
    bool mouse_down = false;
    while (true) {
        event = revyv_event_wait(aslCtx);
        if (event.type == RevyvEventTypeMouseButton && event.mouse_event.button == RevyvMouseButtonTypeLeft
            && event.mouse_event.button_state == RevyvMouseButtonStatePressed) {
            mouse_down = true;
            CefMouseEvent cef_mouse_event;
            cef_mouse_event.x = (int)event.mouse_event.x;
            cef_mouse_event.y = (int)event.mouse_event.y;
            browser->GetHost()->SendMouseClickEvent(cef_mouse_event, MBT_LEFT, false, event.mouse_event.clicks <= 3 ? event.mouse_event.clicks : 3);
        } else if (event.type == RevyvEventTypeMouseButton && event.mouse_event.button == RevyvMouseButtonTypeLeft
            && event.mouse_event.button_state == RevyvMouseButtonStateReleased) {
            mouse_down = false;
            CefMouseEvent cef_mouse_event;
            cef_mouse_event.x = (int)event.mouse_event.x;
            cef_mouse_event.y = (int)event.mouse_event.y;
            browser->GetHost()->SendMouseClickEvent(cef_mouse_event, MBT_LEFT, true, event.mouse_event.clicks <= 3 ? event.mouse_event.clicks : 3);
        } else if (event.type == RevyvEventTypeMouseMove) {
            CefMouseEvent cef_mouse_event;
            if (mouse_down) {
                cef_mouse_event.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;
            }
            cef_mouse_event.x = (int)event.mouse_event.x;
            cef_mouse_event.y = (int)event.mouse_event.y;
            browser->GetHost()->SendMouseMoveEvent(cef_mouse_event, false);
        } else if (event.type == RevyvEventTypeMouseScroll) {
            CefMouseEvent cef_mouse_event;
            cef_mouse_event.x = (int)event.mouse_event.x;
            cef_mouse_event.y = (int)event.mouse_event.y;
            int velocity = 20;
            browser->GetHost()->SendMouseWheelEvent(cef_mouse_event, event.mouse_event.scroll_x * velocity, event.mouse_event.scroll_y * velocity);
        } else if (event.type == RevyvEventTypeText) {
            for (size_t i = 0; i < event.text_event.text_size; i++) {
                CefKeyEvent cefEvent;
                cefEvent.type = KEYEVENT_CHAR;
                cefEvent.character = (unsigned char)event.text_event.text[0];
                browser->GetHost()->SendKeyEvent(cefEvent);
            }
        } else if (event.type == RevyvEventTypeKey) {
            CefKeyEvent cef_key_event;
            cef_key_event.is_system_key = false;
            cef_key_event.modifiers = get_chromium_key_modifiers(event.key_event.keymod);
            if (event.key_event.state == RevyvKeyStatePressed) {
                cef_key_event.type = KEYEVENT_KEYDOWN;
            } else if (event.key_event.state == RevyvKeyStateReleased) {
                cef_key_event.type = KEYEVENT_KEYUP;
            }
            auto code = get_chromium_keyboard_code(event.key_event.scancode);
            if (code != VKEY_UNKNOWN) {
                cef_key_event.windows_key_code = code;
                browser->GetHost()->SendKeyEvent(cef_key_event);
            }
            /* Apparently, just sending a VKEY_RETURN is not enough, and not working
             * properly in some cases. That's why we send here an additional KEYEVENT_CHAR. */
            if (event.key_event.state == RevyvKeyStatePressed && event.key_event.scancode == REVYV_SCANCODE_RETURN) {
                CefKeyEvent cef_char_event;
                cef_char_event.type = KEYEVENT_CHAR;
                cef_char_event.character = '\r';
                browser->GetHost()->SendKeyEvent(cef_char_event);
            }
        }
    }
}

uint32_t EventThread::get_chromium_key_modifiers(int32_t keymod)
{
    uint32_t result = 0;
    if ((keymod & REVYV_KMOD_LSHIFT) || (keymod & REVYV_KMOD_RSHIFT)) {
        result |= EVENTFLAG_SHIFT_DOWN;
    }
    if ((keymod & REVYV_KMOD_LCTRL) || (keymod & REVYV_KMOD_RCTRL)) {
        result |= EVENTFLAG_CONTROL_DOWN;
    }
    if ((keymod & REVYV_KMOD_ALT)) {
        result |= EVENTFLAG_ALT_DOWN;
    }
    return result;
}

int EventThread::get_chromium_keyboard_code(int32_t scancode)
{
    switch (scancode) {
    case REVYV_SCANCODE_A:
        return VKEY_A;
    case REVYV_SCANCODE_B:
        return VKEY_B;
    case REVYV_SCANCODE_C:
        return VKEY_C;
    case REVYV_SCANCODE_D:
        return VKEY_D;
    case REVYV_SCANCODE_E:
        return VKEY_E;
    case REVYV_SCANCODE_F:
        return VKEY_F;
    case REVYV_SCANCODE_G:
        return VKEY_G;
    case REVYV_SCANCODE_H:
        return VKEY_H;
    case REVYV_SCANCODE_I:
        return VKEY_I;
    case REVYV_SCANCODE_J:
        return VKEY_J;
    case REVYV_SCANCODE_K:
        return VKEY_K;
    case REVYV_SCANCODE_L:
        return VKEY_L;
    case REVYV_SCANCODE_M:
        return VKEY_M;
    case REVYV_SCANCODE_N:
        return VKEY_N;
    case REVYV_SCANCODE_O:
        return VKEY_O;
    case REVYV_SCANCODE_P:
        return VKEY_P;
    case REVYV_SCANCODE_Q:
        return VKEY_Q;
    case REVYV_SCANCODE_R:
        return VKEY_R;
    case REVYV_SCANCODE_S:
        return VKEY_S;
    case REVYV_SCANCODE_T:
        return VKEY_T;
    case REVYV_SCANCODE_U:
        return VKEY_U;
    case REVYV_SCANCODE_V:
        return VKEY_V;
    case REVYV_SCANCODE_W:
        return VKEY_W;
    case REVYV_SCANCODE_X:
        return VKEY_X;
    case REVYV_SCANCODE_Y:
        return VKEY_Y;
    case REVYV_SCANCODE_Z:
        return VKEY_Z;

    case REVYV_SCANCODE_COMMA:
        return VKEY_OEM_COMMA;
    case REVYV_SCANCODE_PERIOD:
        return VKEY_OEM_PERIOD;
    case REVYV_SCANCODE_GRAVE:
        return VKEY_OEM_3;
    case REVYV_SCANCODE_MINUS:
        return VKEY_OEM_MINUS;
    case REVYV_SCANCODE_EQUALS:
        return VKEY_OEM_PLUS;
    case REVYV_SCANCODE_LEFTBRACKET:
        return VKEY_OEM_4;
    case REVYV_SCANCODE_RIGHTBRACKET:
        return VKEY_OEM_6;
    case REVYV_SCANCODE_SEMICOLON:
        return VKEY_OEM_1;
    case REVYV_SCANCODE_APOSTROPHE:
        return VKEY_OEM_7;

    case REVYV_SCANCODE_1:
        return VKEY_1;
    case REVYV_SCANCODE_2:
        return VKEY_2;
    case REVYV_SCANCODE_3:
        return VKEY_3;
    case REVYV_SCANCODE_4:
        return VKEY_4;
    case REVYV_SCANCODE_5:
        return VKEY_5;
    case REVYV_SCANCODE_6:
        return VKEY_6;
    case REVYV_SCANCODE_7:
        return VKEY_7;
    case REVYV_SCANCODE_8:
        return VKEY_8;
    case REVYV_SCANCODE_9:
        return VKEY_9;
    case REVYV_SCANCODE_0:
        return VKEY_0;

    case REVYV_SCANCODE_F1:
        return VKEY_F1;
    case REVYV_SCANCODE_F2:
        return VKEY_F2;
    case REVYV_SCANCODE_F3:
        return VKEY_F3;
    case REVYV_SCANCODE_F4:
        return VKEY_F4;
    case REVYV_SCANCODE_F5:
        return VKEY_F5;
    case REVYV_SCANCODE_F6:
        return VKEY_F6;
    case REVYV_SCANCODE_F7:
        return VKEY_F7;
    case REVYV_SCANCODE_F8:
        return VKEY_F8;
    case REVYV_SCANCODE_F9:
        return VKEY_F9;
    case REVYV_SCANCODE_F10:
        return VKEY_F10;
    case REVYV_SCANCODE_F11:
        return VKEY_F11;
    case REVYV_SCANCODE_F12:
        return VKEY_F12;

    case REVYV_SCANCODE_KP_1:
        return VKEY_NUMPAD1;
    case REVYV_SCANCODE_KP_2:
        return VKEY_NUMPAD2;
    case REVYV_SCANCODE_KP_3:
        return VKEY_NUMPAD3;
    case REVYV_SCANCODE_KP_4:
        return VKEY_NUMPAD4;
    case REVYV_SCANCODE_KP_5:
        return VKEY_NUMPAD5;
    case REVYV_SCANCODE_KP_6:
        return VKEY_NUMPAD6;
    case REVYV_SCANCODE_KP_7:
        return VKEY_NUMPAD7;
    case REVYV_SCANCODE_KP_8:
        return VKEY_NUMPAD8;
    case REVYV_SCANCODE_KP_9:
        return VKEY_NUMPAD9;
    case REVYV_SCANCODE_KP_0:
        return VKEY_NUMPAD0;

    case REVYV_SCANCODE_NUMLOCKCLEAR:
        return VKEY_NUMLOCK;
    case REVYV_SCANCODE_KP_DIVIDE:
        return VKEY_DIVIDE;
    case REVYV_SCANCODE_KP_MULTIPLY:
        return VKEY_MULTIPLY;
    case REVYV_SCANCODE_KP_MINUS:
        return VKEY_SUBTRACT;
    case REVYV_SCANCODE_KP_PLUS:
        return VKEY_ADD;
    case REVYV_SCANCODE_KP_ENTER:
        return VKEY_RETURN;
    case REVYV_SCANCODE_KP_DECIMAL:
        return VKEY_DECIMAL;
    case REVYV_SCANCODE_KP_PERIOD:
        return VKEY_OEM_PERIOD;

    case REVYV_SCANCODE_CLEAR:
        return VKEY_CLEAR;
    case REVYV_SCANCODE_POWER:
        return VKEY_POWER;
    case REVYV_SCANCODE_PAGEUP:
        return VKEY_PRIOR;
    case REVYV_SCANCODE_PAGEDOWN:
        return VKEY_NEXT;
    case REVYV_SCANCODE_PAUSE:
        return VKEY_PAUSE;
    case REVYV_SCANCODE_PRINTSCREEN:
        return VKEY_PRINT;
    case REVYV_SCANCODE_SLASH:
        return VKEY_OEM_2;
    case REVYV_SCANCODE_BACKSLASH:
        return VKEY_OEM_5;
    case REVYV_SCANCODE_SPACE:
        return VKEY_SPACE;
    case REVYV_SCANCODE_CAPSLOCK:
        return VKEY_CAPITAL;
    case REVYV_SCANCODE_LGUI:
    case REVYV_SCANCODE_RGUI:
        return VKEY_COMMAND;
    case REVYV_SCANCODE_LSHIFT:
    case REVYV_SCANCODE_RSHIFT:
        return VKEY_SHIFT;
    case REVYV_SCANCODE_LCTRL:
    case REVYV_SCANCODE_RCTRL:
        return VKEY_CONTROL;
    case REVYV_SCANCODE_LALT:
    case REVYV_SCANCODE_RALT:
        return VKEY_MENU;
    case REVYV_SCANCODE_ESCAPE:
        return VKEY_ESCAPE;
    case REVYV_SCANCODE_BACKSPACE:
        return VKEY_BACK;
    case REVYV_SCANCODE_RETURN:
        return VKEY_RETURN;
    case REVYV_SCANCODE_TAB:
        return VKEY_TAB;
    case REVYV_SCANCODE_DELETE:
        return VKEY_DELETE;
    case REVYV_SCANCODE_INSERT:
        return VKEY_INSERT;
    case REVYV_SCANCODE_END:
        return VKEY_END;
    case REVYV_SCANCODE_HOME:
        return VKEY_HOME;
    case REVYV_SCANCODE_UP:
        return VKEY_UP;
    case REVYV_SCANCODE_DOWN:
        return VKEY_DOWN;
    case REVYV_SCANCODE_LEFT:
        return VKEY_LEFT;
    case REVYV_SCANCODE_RIGHT:
        return VKEY_RIGHT;

    default:
        return VKEY_UNKNOWN;
    }
}
