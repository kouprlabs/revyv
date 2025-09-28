#ifndef REVYV_CONNECTOR_H
#define REVYV_CONNECTOR_H

#include "socket.h"
#include <cerrno>
#include <compositor/types.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <zmq.hpp>

namespace revyv {

typedef struct
{
    uint32_t id;
    int shared_memory_id;
    unsigned char* shared_memory;
} Window;

class Connector {
public:
    Connector();

    ~Connector();

    uint32_t window_create(unsigned char* data, uint64_t size, double x, double y, double width, double height, WindowRasterType raster_type);

    void window_update_pixels(uint32_t window_id, unsigned char* data, uint64_t size, double x, double y, double width, double height);

    void window_resize(uint32_t window_id, unsigned char* data, uint64_t size, double width, double height);

    void window_change_visiblity(uint32_t window_id, bool visible);

    void window_bring_to_front(uint32_t window_id);

    void window_move(uint32_t window_id, double x, double y);

    void window_destroy(uint32_t window_id);

    std::shared_ptr<Event> event_wait();

private:
    std::shared_ptr<Socket> _compositor;
    std::shared_ptr<Socket> _listener;
    std::shared_ptr<Socket> _publisher;
    std::unordered_map<uint32_t, Window> _windows;
};
}

#endif
