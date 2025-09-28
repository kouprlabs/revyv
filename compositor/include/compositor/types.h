#ifndef REVYV_COMPOSITOR_TYPES_H
#define REVYV_COMPOSITOR_TYPES_H

#include <csignal>
#include <cstdint>
#include <string>

namespace revyv {

typedef enum : uint8_t {
    RequestTypeClientRegister = 1,
    RequestTypeClientUnregister = 2,
    RequestTypeWindowCreate = 3,
    RequestTypeWindowMove = 4,
    RequestTypeWindowResize = 5,
    RequestTypeWindowUpdatePixels = 6,
    RequestTypeWindowSetVisibility = 7,
    RequestTypeWindowDestroy = 8,
    RequestTypeWindowBringToFront = 9,
} RequestType;

typedef enum : uint8_t {
    WindowRasterRGBA = 1,
    WindowRasterARGB = 2,
} WindowRasterType;

const uint32_t PUBLISHER_PORT_BASE = 10000;
const uint32_t LISTENER_PORT_BASE = 20000;

typedef struct
{
    pid_t pid;
    uint8_t type;
    uint32_t window_id;
    double field0;
    double field1;
    double field2;
    double field3;
    int8_t field4;
    uint64_t field5;
    uint64_t field6;
    uint64_t field7;
} ListenerPayload;

class ListenerBasePayload {
public:
    ListenerBasePayload() = default;

    virtual ~ListenerBasePayload() = default;

    explicit ListenerBasePayload(const ListenerPayload& p)
        : _payload(p)
    {
    }

    [[nodiscard]] ListenerPayload get_payload() const { return _payload; }

    [[nodiscard]] uint8_t get_type() const { return _payload.type; }

    [[nodiscard]] pid_t get_pid() const { return _payload.pid; }

    [[nodiscard]] uint32_t get_window_id() const { return _payload.window_id; }

protected:
    ListenerPayload _payload {};
};

class RegisterClientPayload : public ListenerBasePayload {
public:
    explicit RegisterClientPayload(const ListenerPayload& p)
        : ListenerBasePayload(p)
    {
    }

    explicit RegisterClientPayload(pid_t pid)
    {
        _payload.type = RequestTypeClientRegister;
        _payload.pid = pid;
    }
};

class ClientUnregisterPayload : public ListenerBasePayload {
public:
    explicit ClientUnregisterPayload(ListenerPayload p)
        : ListenerBasePayload(p)
    {
    }

    explicit ClientUnregisterPayload()
    {
        _payload.type = RequestTypeClientUnregister;
    }
};

class WindowCreatePayload : public ListenerBasePayload {
public:
    explicit WindowCreatePayload(const ListenerPayload& p)
        : ListenerBasePayload(p)
    {
    }

    WindowCreatePayload(
        uint32_t window_id,
        double x,
        double y,
        double width,
        double height,
        WindowRasterType raster_type,
        uint64_t data_size,
        int shared_memory_id)
    {
        _payload.type = RequestTypeWindowCreate;
        _payload.window_id = window_id;
        _payload.field0 = x;
        _payload.field1 = y;
        _payload.field2 = width;
        _payload.field3 = height;
        _payload.field4 = raster_type;
        _payload.field5 = data_size;
        _payload.field6 = shared_memory_id;
    }

    [[nodiscard]] double get_x() const { return _payload.field0; }

    [[nodiscard]] double get_y() const { return _payload.field1; }

    [[nodiscard]] double get_width() const { return _payload.field2; }

    [[nodiscard]] double get_height() const { return _payload.field3; }

    [[nodiscard]] WindowRasterType get_raster_type() const { return (WindowRasterType)_payload.field4; }

    [[nodiscard]] uint64_t get_data_size() const { return _payload.field5; }

    [[nodiscard]] int get_shared_memory_id() const { return _payload.field6; }
};

class WindowUpdatePixelsPayload : public ListenerBasePayload {
public:
    explicit WindowUpdatePixelsPayload(const ListenerPayload& p)
        : ListenerBasePayload(p)
    {
    }

    WindowUpdatePixelsPayload(
        uint32_t window_id,
        double x,
        double y,
        double width,
        double height,
        uint64_t data_size,
        bool compressed,
        uint64_t compressed_size,
        int shared_memory_id)
    {
        _payload.type = RequestTypeWindowUpdatePixels;
        _payload.window_id = window_id;
        _payload.field0 = x;
        _payload.field1 = y;
        _payload.field2 = width;
        _payload.field3 = height;
        _payload.field5 = data_size;
        _payload.field4 = compressed;
        _payload.field6 = compressed_size;
        _payload.field7 = shared_memory_id;
    }

    [[nodiscard]] double get_x() const { return _payload.field0; }

    [[nodiscard]] double get_y() const { return _payload.field1; }

    [[nodiscard]] double get_width() const { return _payload.field2; }

    [[nodiscard]] double get_height() const { return _payload.field3; }

    [[nodiscard]] uint64_t get_data_size() const { return _payload.field5; }

    [[nodiscard]] uint64_t is_compressed() const { return _payload.field4; }

    [[nodiscard]] uint64_t get_compressed_size() const { return _payload.field6; }

    [[nodiscard]] int get_shared_memory_id() const { return _payload.field7; }
};

class WindowResizePayload : public ListenerBasePayload {
public:
    explicit WindowResizePayload(const ListenerPayload& p)
        : ListenerBasePayload(p)
    {
    }

    WindowResizePayload(
        uint32_t window_id,
        double width,
        double height,
        uint64_t data_size,
        int shared_memory_id)
    {
        _payload.type = RequestTypeWindowResize;
        _payload.window_id = window_id;
        _payload.field0 = width;
        _payload.field1 = height;
        _payload.field5 = data_size;
        _payload.field6 = shared_memory_id;
    }

    [[nodiscard]] double get_width() const { return _payload.field0; }

    [[nodiscard]] double get_height() const { return _payload.field1; }

    [[nodiscard]] uint64_t get_data_size() const { return _payload.field5; }

    [[nodiscard]] int get_shared_memory_id() const { return _payload.field6; }
};

class WindowSetVisibilityPayload : public ListenerBasePayload {
public:
    explicit WindowSetVisibilityPayload(const ListenerPayload& p)
        : ListenerBasePayload(p)
    {
    }

    WindowSetVisibilityPayload(uint32_t window_id, bool visible)
    {
        _payload.type = RequestTypeWindowSetVisibility;
        _payload.window_id = window_id;
        _payload.field0 = visible;
    }

    [[nodiscard]] bool is_visible() const { return (bool)_payload.field0; }
};

class WindowBringToFrontPayload : public ListenerBasePayload {
public:
    explicit WindowBringToFrontPayload(const ListenerPayload& p)
        : ListenerBasePayload(p)
    {
    }

    explicit WindowBringToFrontPayload(uint32_t window_id)
    {
        _payload.type = RequestTypeWindowBringToFront;
        _payload.window_id = window_id;
    }
};

class WindowMovePayload : public ListenerBasePayload {
public:
    explicit WindowMovePayload(const ListenerPayload& p)
        : ListenerBasePayload(p)
    {
    }

    WindowMovePayload(uint32_t window_id, double x, double y)
    {
        _payload.type = RequestTypeWindowMove;
        _payload.window_id = window_id;
        _payload.field0 = x;
        _payload.field1 = y;
    }

    [[nodiscard]] double get_x() const { return _payload.field0; }

    [[nodiscard]] double get_y() const { return _payload.field1; }
};

class WindowDestroyPayload : public ListenerBasePayload {
public:
    explicit WindowDestroyPayload(const ListenerPayload& p)
        : ListenerBasePayload(p)
    {
    }

    explicit WindowDestroyPayload(uint32_t window_id)
    {
        _payload.type = RequestTypeWindowMove;
        _payload.window_id = window_id;
    }
};

typedef enum : uint8_t {
    EventTypeUndefined = 0,
    EventTypeMouseMove = 1,
    EventTypeMouseButton = 2,
    EventTypeMouseScroll = 3,
    EventTypeKey = 4,
    EventTypeText = 5,
    EventTypeQuit = 6,
} EventType;

typedef struct
{
    uint32_t window_id;
    uint8_t type;
    double field0;
    double field1;
    double field2;
    double field3;
    uint8_t field4;
    uint8_t field5;
    uint8_t field6;
    uint32_t field7;
} PublisherPayload;

typedef enum {
    MouseButtonTypeUndefined = 0,
    MouseButtonTypeRight = 1,
    MouseButtonTypeLeft = 2,
    MouseButtonTypeMiddle = 3,
} MouseButtonType;

typedef enum {
    MouseButtonStateUndefined = 0,
    MouseButtonStatePressed = 1,
    MouseButtonStateReleased = 2,
} MouseButtonState;

typedef enum {
    KeyStateUndefined = 0,
    KeyStatePressed = 1,
    KeyStateReleased = 2,
} KeyState;

class Event {
public:
    explicit Event(EventType type)
        : _type(type)
    {
    }

    explicit Event(const PublisherPayload& payload)
        : _window_id(payload.window_id)
        , _timestamp(payload.field7)
        , _type((EventType)payload.type)
    {
    }

    [[nodiscard]] uint32_t get_timestamp() const { return _timestamp; }

    void set_timestamp(uint32_t timestamp) { _timestamp = timestamp; }

    [[nodiscard]] EventType get_type() const { return _type; }

    void set_type(EventType type) { _type = type; }

    [[nodiscard]] uint32_t get_window_id() const { return _window_id; }

    void set_window_id(uint32_t window_id) { _window_id = window_id; }

    [[nodiscard]] virtual PublisherPayload translate()
    {
        PublisherPayload p;
        p.window_id = get_window_id();
        p.type = get_type();
        p.field0 = 0;
        p.field1 = 0;
        p.field2 = 0;
        p.field3 = 0;
        p.field4 = 0;
        p.field5 = 0;
        p.field7 = get_timestamp();
        return p;
    }

private:
    EventType _type = EventTypeUndefined;
    uint32_t _window_id {};
    uint32_t _timestamp {};
};

class QuitEvent : public Event {
public:
    QuitEvent()
        : Event(EventTypeQuit)
    {
    }
};

class MouseEvent : public Event {
public:
    explicit MouseEvent(const PublisherPayload& payload)
        : Event(payload)
    {
    }

    explicit MouseEvent(EventType type)
        : Event(type)
    {
    }

    [[nodiscard]] double get_x() const { return _x; }

    void set_x(double x) { _x = x; }

    [[nodiscard]] double get_y() const { return _y; }

    void set_y(double y) { _y = y; }

    [[nodiscard]] double get_window_x() const { return _windowX; }

    void set_window_x(double windowX) { _windowX = windowX; }

    [[nodiscard]] double get_window_y() const { return _windowY; }

    void set_window_y(double windowY) { _windowY = windowY; }

private:
    double _x {};
    double _y {};
    double _windowX {};
    double _windowY {};
};

class MouseMoveEvent : public MouseEvent {
public:
    MouseMoveEvent()
        : MouseEvent(EventTypeMouseMove)
    {
    }

    explicit MouseMoveEvent(const PublisherPayload& payload)
        : MouseEvent(payload)
    {
        set_window_x(payload.field0);
        set_window_y(payload.field1);
        set_x(payload.field2);
        set_y(payload.field3);
    }

    [[nodiscard]] PublisherPayload translate() override
    {
        PublisherPayload p;
        p.window_id = get_window_id();
        p.type = get_type();
        p.field0 = get_window_x();
        p.field1 = get_window_y();
        p.field2 = get_x();
        p.field3 = get_y();
        p.field4 = 0;
        p.field5 = 0;
        p.field7 = get_timestamp();
        return p;
    }
};

class MouseButtonEvent : public MouseEvent {
public:
    MouseButtonEvent()
        : MouseEvent(EventTypeMouseButton)
    {
    }

    explicit MouseButtonEvent(const PublisherPayload& payload)
        : MouseEvent(payload)
        , _button(payload.field4)
        , _state(payload.field5)
        , _clicks(payload.field6)
    {
        set_window_x(payload.field0);
        set_window_y(payload.field1);
        set_x(payload.field2);
        set_y(payload.field3);
    }

    [[nodiscard]] uint8_t get_button() const { return _button; }

    void set_button(uint8_t button) { _button = button; }

    [[nodiscard]] uint8_t get_state() const { return _state; }

    void set_state(uint8_t state) { _state = state; }

    [[nodiscard]] uint8_t get_clicks() const { return _clicks; }

    void setClicks(uint8_t clicks) { _clicks = clicks; }

    [[nodiscard]] PublisherPayload translate() override
    {
        PublisherPayload p;
        p.window_id = get_window_id();
        p.type = get_type();
        p.field0 = get_window_x();
        p.field1 = get_window_y();
        p.field2 = get_x();
        p.field3 = get_y();
        p.field4 = get_button();
        p.field5 = get_state();
        p.field6 = get_clicks();
        p.field7 = get_timestamp();
        return p;
    }

private:
    uint8_t _button = MouseButtonTypeUndefined;
    uint8_t _state = MouseButtonStateUndefined;
    uint8_t _clicks {};
};

class MouseScrollEvent : public MouseEvent {
public:
    MouseScrollEvent()
        : MouseEvent(EventTypeMouseScroll)
    {
    }

    explicit MouseScrollEvent(const PublisherPayload& payload)
        : MouseEvent(payload)
        , _scrollX((int32_t)payload.field2)
        , _scrollY((int32_t)payload.field3)
    {
        set_window_x(payload.field0);
        set_window_y(payload.field1);
    }

    [[nodiscard]] int32_t get_scroll_x() const { return _scrollX; }

    void setScrollX(int32_t scrollX) { _scrollX = scrollX; }

    [[nodiscard]] int32_t get_scroll_y() const { return _scrollY; }

    void setScrollY(int32_t scrollY) { _scrollY = scrollY; }

    [[nodiscard]] bool get_flipped() const { return _flipped; }

    void set_flipped(bool flipped) { _flipped = flipped; }

    [[nodiscard]] PublisherPayload translate() override
    {
        PublisherPayload p;
        p.window_id = get_window_id();
        p.type = get_type();
        p.field0 = get_window_x();
        p.field1 = get_window_y();
        p.field2 = get_scroll_x();
        p.field3 = get_scroll_y();
        p.field4 = 0;
        p.field5 = 0;
        p.field7 = get_timestamp();
        return p;
    }

private:
    int32_t _scrollX {};
    int32_t _scrollY {};
    bool _flipped {};
};

class TextEvent : public Event {
public:
    explicit TextEvent(const PublisherPayload& payload)
        : Event(payload)
    {
    }

    TextEvent()
        : Event(EventTypeText)
    {
    }

    [[nodiscard]] const std::string& get_text() const { return _text; }

    void set_text(const std::string& text) { _text = text; }

    [[nodiscard]] PublisherPayload translate() override
    {
        PublisherPayload p;
        p.window_id = get_window_id();
        p.type = get_type();
        p.field0 = (double)(get_text().size() + 1);
        p.field1 = 0;
        p.field2 = 0;
        p.field3 = 0;
        p.field4 = 0;
        p.field5 = 0;
        p.field7 = get_timestamp();
        return p;
    }

private:
    std::string _text;
};

class KeyEvent : public Event {
public:
    KeyEvent()
        : Event(EventTypeKey)
    {
    }

    explicit KeyEvent(const PublisherPayload& payload)
        : Event(payload)
        , _keycode((int32_t)payload.field0)
        , _scancode((int32_t)payload.field1)
        , _keymod((int32_t)payload.field2)
        , _state((int8_t)payload.field3)
        , _repeat((int32_t)payload.field4)
    {
    }

    [[nodiscard]] int32_t get_scancode() const { return _scancode; }

    void set_scancode(int32_t scancode) { _scancode = scancode; }

    [[nodiscard]] int32_t get_keycode() const { return _keycode; }

    void set_keycode(int32_t keycode) { _keycode = keycode; }

    [[nodiscard]] int32_t get_keymod() const { return _keymod; }

    void set_keymod(int32_t keymod) { _keymod = keymod; }

    [[nodiscard]] int8_t get_state() const { return _state; }

    void set_state(int8_t state) { _state = state; }

    [[nodiscard]] bool get_repeat() const { return _repeat; }

    void set_repeat(bool repeat) { _repeat = repeat; }

    [[nodiscard]] PublisherPayload translate() override
    {
        PublisherPayload p {};
        p.window_id = get_window_id();
        p.type = get_type();
        p.field0 = get_keycode();
        p.field1 = get_scancode();
        p.field2 = get_keymod();
        p.field3 = get_state();
        p.field4 = get_repeat();
        p.field5 = 0;
        p.field7 = get_timestamp();
        return p;
    }

private:
    int32_t _scancode {};
    int32_t _keycode {};
    int32_t _keymod {};
    int8_t _state = KeyStateUndefined;
    bool _repeat {};
};
}

#endif
