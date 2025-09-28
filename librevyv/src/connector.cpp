#include "connector.h"
#include "compressor.h"
#include "socket.h"
#include <lzo/lzo1x.h>
#include <sstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#define EXPORT __attribute__((visibility("default")))

using namespace revyv;

Connector::Connector()
{
    if (lzo_init() != LZO_E_OK) {
        throw std::runtime_error("lzo_init() failed");
    }

    auto pid = getpid();

    _publisher = std::make_shared<Socket>(
        SocketBind,
        "ipc:///tmp/revyv-publisher-" + std::to_string(PUBLISHER_PORT_BASE + pid));
    std::cout << "Publisher listening on " << _publisher->get_url() << std::endl;

    _compositor = std::make_shared<Socket>(SocketConnect, "ipc:///tmp/revyv-compositor");
    std::cout << "Connected to compositor on " << _compositor->get_url()
              << std::endl;

    _compositor->send_listener_payload(
        RegisterClientPayload(getpid()).get_payload());
    std::cout << "Registered with PID " << pid << std::endl;

    _listener = std::make_shared<Socket>(
        SocketConnect,
        "ipc:///tmp/revyv-listener-" + std::to_string(LISTENER_PORT_BASE + pid));
    std::cout << "Listener connected on " << _listener->get_url() << std::endl;
}

Connector::~Connector() = default;

uint32_t Connector::window_create(unsigned char* data, size_t size, double x, double y, double width, double height, WindowRasterType raster_type)
{
    Window w {};
    w.id = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + getpid();
    w.shared_memory_id = shmget(IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    w.shared_memory = (unsigned char*)shmat(w.shared_memory_id, 0, 0);
    _windows[w.id] = w;

    std::memcpy(w.shared_memory, data, size);
    _listener->send_listener_payload(WindowCreatePayload(w.id, x, y, width, height, raster_type, size, w.shared_memory_id).get_payload());

    return w.id;
}

void Connector::window_update_pixels(uint32_t window_id, unsigned char* data, size_t size, double x, double y, double width, double height)
{
    auto w = _windows[window_id];
    try {
        Compressor compressor(data, size);
        std::memcpy(w.shared_memory, compressor.getData().get(), compressor.getSize());
        _listener->send_listener_payload(WindowUpdatePixelsPayload(window_id, x, y, width, height, size, true, compressor.getSize(), w.shared_memory_id).get_payload());
    } catch (FailedToCompressDataError& e) {
        /* We failed to compress data, send it uncompressed. */
        std::memcpy(w.shared_memory, data, size);
        _listener->send_listener_payload(WindowUpdatePixelsPayload(window_id, x, y, width, height, size, false, size, w.shared_memory_id).get_payload());
    }
}

void Connector::window_resize(uint32_t window_id, unsigned char* data, uint64_t size, double width, double height)
{
    auto w = _windows[window_id];
    _listener->send_listener_payload(WindowResizePayload(w.id, width, height, size, w.shared_memory_id).get_payload());
    _listener->send_data(data, size);
}

void Connector::window_change_visiblity(uint32_t window_id, bool visible)
{
    _listener->send_listener_payload(WindowSetVisibilityPayload(window_id, visible).get_payload());
}

void Connector::window_bring_to_front(uint32_t window_id)
{
    _listener->send_listener_payload(WindowBringToFrontPayload(window_id).get_payload());
}

void Connector::window_move(uint32_t window_id, double x, double y)
{
    _listener->send_listener_payload(WindowMovePayload(window_id, x, y).get_payload());
}

void Connector::window_destroy(uint32_t window_id)
{
    auto w = _windows[window_id];
    _listener->send_listener_payload(WindowDestroyPayload(w.id).get_payload());
    shmdt(w.shared_memory);
    _windows.erase(_windows.find(w.id));
}

std::shared_ptr<Event> Connector::event_wait()
{
    auto p = _publisher->recv_publisher_payload();
    if (p.type == EventTypeMouseMove) {
        return std::make_shared<MouseMoveEvent>(p);
    } else if (p.type == EventTypeMouseButton) {
        return std::make_shared<MouseButtonEvent>(p);
    } else if (p.type == EventTypeMouseScroll) {
        return std::make_shared<MouseScrollEvent>(p);
    } else if (p.type == EventTypeKey) {
        return std::make_shared<KeyEvent>(p);
    } else if (p.type == EventTypeText) {
        auto text = _publisher->recv_rext((size_t)p.field0);
        auto event = std::make_shared<TextEvent>(p);
        event->set_text(text.get());
        return event;
    }
    return nullptr;
}
