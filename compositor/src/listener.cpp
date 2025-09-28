#include "listener.h"
#include "error.h"
#include "sdl_compositor.h"
#include "sdl_window.h"
#include "server.h"
#include <compositor/types.h>
#include <lzo/lzo1x.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

using namespace revyv;

Listener::Listener(pid_t pid)
    : _pid(pid)
{
    _url = "ipc:///tmp/revyv-listener-" + std::to_string(LISTENER_PORT_BASE + _pid);
    _ctx = std::make_shared<zmq::context_t>();
    _socket = std::make_shared<zmq::socket_t>(*_ctx, ZMQ_PULL);
    _socket->set(zmq::sockopt::sndtimeo, 10000);
    _socket->set(zmq::sockopt::rcvtimeo, 10000);
    _socket->bind(_url.c_str());
    _thread = std::make_shared<std::thread>(Listener::listener_thread, this);
    _thread->detach();
}

void Listener::shutdown()
{
    _running = false;
}

void Listener::listener_thread(Listener* listener)
{
    std::cout << "Started listener thread for PID " << listener->_pid << " on " << listener->_url
              << std::endl;
    while (listener->_running) {
        try {
            ListenerPayload p {};
            auto result = listener->_socket->recv(zmq::mutable_buffer(&p, sizeof(ListenerPayload)), zmq::recv_flags::none);
            if (result.has_value()) {
                listener->process_message(p);
            }
        } catch (std::exception& e) {
            std::cout << __func__ << ": " << e.what() << std::endl;
        }
    }
    listener->_socket->close();
    std::cout << "Stopped listener thread for PID " << listener->_pid << std::endl;
}

void Listener::process_message(const ListenerPayload& p)
{
    try {
        if (p.type == RequestTypeWindowCreate) {
            window_create(p);
        } else if (p.type == RequestTypeWindowMove) {
            window_move(p);
        } else if (p.type == RequestTypeWindowSetVisibility) {
            window_set_visibility(p);
        } else if (p.type == RequestTypeWindowBringToFront) {
            window_bring_to_front(p);
        } else if (p.type == RequestTypeWindowUpdatePixels) {
            window_update_pixels(p);
        } else if (p.type == RequestTypeWindowResize) {
            window_resize(p);
        } else if (p.type == RequestTypeWindowDestroy) {
            window_destroy(p);
        } else if (p.type == RequestTypeClientUnregister) {
            _running = false;
        }
    } catch (std::exception& e) {
        std::cout << __func__ << ": " << e.what() << std::endl;
    }
}

void Listener::window_create(const ListenerPayload& p)
{
    auto payload = WindowCreatePayload(p);

    auto data = std::shared_ptr<unsigned char[]>(new unsigned char[payload.get_data_size()]);
    unsigned char* shared_memory = (unsigned char*)shmat(payload.get_shared_memory_id(), 0, 0);
    std::memcpy(data.get(), shared_memory, payload.get_data_size());

    auto compositor = Server::get_shared_instance()->get_compositor();
    auto sdl_compositor = std::dynamic_pointer_cast<SDLCompositor>(compositor);
    auto window = std::make_shared<SDLWindow>(_pid, payload.get_window_id(), payload.get_raster_type(), sdl_compositor->get_renderer());
    window->create(data, payload.get_data_size(), make_rect(payload.get_x(), payload.get_y(), payload.get_width(), payload.get_height()));
    window->set_shared_memory_id(payload.get_shared_memory_id());
    window->set_shared_memory_address(shared_memory);
    compositor->add_window(window);
}

void Listener::window_update_pixels(const ListenerPayload& p)
{
    auto payload = WindowUpdatePixelsPayload(p);
    auto window = Server::get_shared_instance()->get_compositor()->find_window(payload.get_window_id());
    if (window.expired() || window.lock() == nullptr) {
        return;
    }

    auto size = payload.is_compressed() ? payload.get_compressed_size() : payload.get_data_size();
    auto buffer = std::shared_ptr<unsigned char[]>(new unsigned char[size]);
    std::memcpy(buffer.get(), window.lock()->get_shared_memory_address(), size);

    std::shared_ptr<unsigned char[]> data;
    if (payload.is_compressed()) {
        data = std::shared_ptr<unsigned char[]>(new unsigned char[payload.get_data_size()]);
        auto new_size = payload.get_data_size();
        auto decompress_result = lzo1x_decompress(buffer.get(), payload.get_compressed_size(), data.get(), &new_size, nullptr);
        if (decompress_result != LZO_E_OK || new_size != payload.get_data_size()) {
            return;
        }
    } else {
        data = buffer;
    }
    window.lock()->update_pixels(data, payload.get_data_size(), make_rect(payload.get_x(), payload.get_y(), payload.get_width(), payload.get_height()));
}

void Listener::window_resize(const ListenerPayload& p)
{
    auto payload = WindowResizePayload(p);
    auto window = Server::get_shared_instance()->get_compositor()->find_window(payload.get_window_id());
    if (window.expired() || window.lock() == nullptr) {
        return;
    }
    auto data = std::shared_ptr<unsigned char[]>(new unsigned char[payload.get_data_size()]);
    std::memcpy(data.get(), window.lock()->get_shared_memory_address(), payload.get_data_size());
    window.lock()->resize(data, payload.get_data_size(), make_size(payload.get_width(), payload.get_height()));
}

void Listener::window_set_visibility(const ListenerPayload& p)
{
    auto payload = WindowSetVisibilityPayload(p);
    auto window = Server::get_shared_instance()->get_compositor()->find_window(payload.get_window_id());
    if (window.expired() || window.lock() == nullptr) {
        return;
    }
    window.lock()->set_visible((bool)payload.is_visible());
}

void Listener::window_move(const ListenerPayload& p)
{
    auto payload = WindowMovePayload(p);
    auto window = Server::get_shared_instance()->get_compositor()->find_window(payload.get_window_id());
    if (window.expired() || window.lock() == nullptr) {
        return;
    }
    auto point = make_point(payload.get_x(), payload.get_y());
    window.lock()->set_location(point);
    window.lock()->move(point);
}

void Listener::window_bring_to_front(const ListenerPayload& p)
{
    auto payload = WindowBringToFrontPayload(p);
    auto window = Server::get_shared_instance()->get_compositor()->find_window(payload.get_window_id());
    if (window.expired() || window.lock() == nullptr) {
        return;
    }
    Server::get_shared_instance()->get_compositor()->window_bring_to_front(window.lock());
}

void Listener::window_destroy(const ListenerPayload& p)
{
    auto payload = WindowDestroyPayload(p);
    auto window = Server::get_shared_instance()->get_compositor()->find_window(payload.get_window_id());
    if (window.expired() || window.lock() == nullptr) {
        return;
    }
    Server::get_shared_instance()->get_compositor()->remove_window_by_id(payload.get_window_id());
}

bool Listener::is_running() const
{
    return _running;
}
