#include "event_thread.h"
#include "chromium_keycodes.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_render_handler.h"
#include "include/cef_task.h"
#include "include/base/cef_ref_counted.h"
#include "include/wrapper/cef_closure_task.h"
#include <functional>
#include <string>
#include <utility>
#include <revyv/revyv.h>

void EventThread::event_thread(CefRefPtr<CefBrowser> browser, void* aslCtx)
{
    if (!browser)
        return;

    RevyvEvent event;
    bool mouse_down = false;

    class BrowserHostTask : public CefTask {
    public:
        BrowserHostTask(CefRefPtr<CefBrowser> browser, std::function<void(CefRefPtr<CefBrowserHost>)> task)
            : browser_(std::move(browser)), task_(std::move(task))
        {
        }

        void Execute() override
        {
            CefRefPtr<CefBrowser> browser = browser_;
            if (!browser)
                return;
            CefRefPtr<CefBrowserHost> host = browser->GetHost();
            if (host && task_)
                task_(host);
        }

    private:
        CefRefPtr<CefBrowser> browser_;
        std::function<void(CefRefPtr<CefBrowserHost>)> task_;

        IMPLEMENT_REFCOUNTING(BrowserHostTask);
    };

    auto post_to_ui = [](CefRefPtr<CefBrowser> target, std::function<void(CefRefPtr<CefBrowserHost>)> task) {
        if (!target)
            return;
        CefPostTask(TID_UI, new BrowserHostTask(target, std::move(task)));
    };

    while (true) {
        event = revyv_event_wait(aslCtx);

        if (event.type == RevyvEventTypeQuit) {
            post_to_ui(browser, [](CefRefPtr<CefBrowserHost> host) { host->CloseBrowser(false); });
            break;
        }

        switch (event.type) {
        case RevyvEventTypeMouseButton: {
            CefBrowserHost::MouseButtonType button = MBT_LEFT;
            switch (event.mouse_event.button) {
            case RevyvMouseButtonTypeRight:
                button = MBT_RIGHT;
                break;
            case RevyvMouseButtonTypeMiddle:
                button = MBT_MIDDLE;
                break;
            case RevyvMouseButtonTypeLeft:
            default:
                button = MBT_LEFT;
                break;
            }

            CefMouseEvent cef_mouse_event;
            cef_mouse_event.x = static_cast<int>(event.mouse_event.x);
            cef_mouse_event.y = static_cast<int>(event.mouse_event.y);

            const int clicks = event.mouse_event.clicks <= 3 ? event.mouse_event.clicks : 3;
            const bool mouse_up = event.mouse_event.button_state == RevyvMouseButtonStateReleased;
            if (event.mouse_event.button == RevyvMouseButtonTypeLeft)
                mouse_down = !mouse_up;

            post_to_ui(browser, [cef_mouse_event, button, mouse_up, clicks](CefRefPtr<CefBrowserHost> host) {
                host->SendMouseClickEvent(cef_mouse_event, button, mouse_up, clicks);
            });
            break;
        }
        case RevyvEventTypeMouseMove: {
            CefMouseEvent cef_mouse_event;
            if (mouse_down)
                cef_mouse_event.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;
            cef_mouse_event.x = static_cast<int>(event.mouse_event.x);
            cef_mouse_event.y = static_cast<int>(event.mouse_event.y);

            post_to_ui(browser, [cef_mouse_event](CefRefPtr<CefBrowserHost> host) {
                host->SendMouseMoveEvent(cef_mouse_event, false);
            });
            break;
        }
        case RevyvEventTypeMouseScroll: {
            CefMouseEvent cef_mouse_event;
            cef_mouse_event.x = static_cast<int>(event.mouse_event.x);
            cef_mouse_event.y = static_cast<int>(event.mouse_event.y);
            const int velocity = 20;
            const int delta_x = event.mouse_event.scroll_x * velocity;
            const int delta_y = event.mouse_event.scroll_y * velocity;

            post_to_ui(browser, [cef_mouse_event, delta_x, delta_y](CefRefPtr<CefBrowserHost> host) {
                host->SendMouseWheelEvent(cef_mouse_event, delta_x, delta_y);
            });
            break;
        }
        case RevyvEventTypeText: {
            if (event.text_event.text && event.text_event.text_size > 0) {
                std::string text(event.text_event.text, event.text_event.text_size);
                post_to_ui(browser, [text](CefRefPtr<CefBrowserHost> host) {
                    for (unsigned char ch : text) {
                        CefKeyEvent cefEvent;
                        cefEvent.type = KEYEVENT_CHAR;
                        cefEvent.character = ch;
                        host->SendKeyEvent(cefEvent);
                    }
                });
            }
            if (event.text_event.text) {
                delete[] event.text_event.text;
                event.text_event.text = nullptr;
            }
            break;
        }
        case RevyvEventTypeKey: {
            CefKeyEvent cef_key_event;
            cef_key_event.is_system_key = false;
            cef_key_event.modifiers = get_chromium_key_modifiers(event.key_event.keymod);
            cef_key_event.type = (event.key_event.state == RevyvKeyStatePressed) ? KEYEVENT_KEYDOWN : KEYEVENT_KEYUP;

            const int windows_key_code = get_chromium_keyboard_code(event.key_event.scancode);
            if (windows_key_code != VKEY_UNKNOWN) {
                cef_key_event.windows_key_code = windows_key_code;
                post_to_ui(browser, [cef_key_event](CefRefPtr<CefBrowserHost> host) {
                    host->SendKeyEvent(cef_key_event);
                });
            }

            if (event.key_event.state == RevyvKeyStatePressed && event.key_event.scancode == REVYV_SCANCODE_RETURN) {
                CefKeyEvent cef_char_event;
                cef_char_event.type = KEYEVENT_CHAR;
                cef_char_event.character = '\r';
                post_to_ui(browser, [cef_char_event](CefRefPtr<CefBrowserHost> host) {
                    host->SendKeyEvent(cef_char_event);
                });
            }
            break;
        }
        default:
            break;
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
