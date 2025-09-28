#include "render_handler.h"
#include <cairo/cairo.h>
#include <revyv/revyv.h>

RenderHandler::RenderHandler(void* revyv, double x, double y, double width,
    double height)
    : _revyv_ctx(revyv)
    , _x(x)
    , _y(y)
    , _width(width)
    , _height(height)
{
}

void RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    rect = CefRect((int)_x, (int)_y, (int)_width, (int)_height);
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirty_rects, const void* buffer, int width, int height)
{
    if (_window_id == 0) {
        _window_id = revyv_window_create(_revyv_ctx, (unsigned char*)buffer, width * height * 4, _x, _y, _width, _height, RevyvWindowRasterARGB);
    } else {
        if (dirty_rects.size() > 1) {
            revyv_window_update(_revyv_ctx, _window_id, (unsigned char*)buffer, width * height * 4, 0, 0, width, height);
        } else if (dirty_rects.size() == 1) {
            CefRect rect = dirty_rects.at(0);
            if (rect.x == 0 && rect.y == 0 && rect.width == width && rect.height == height) {
                size_t data_size = width * height * 4;
                revyv_window_update(_revyv_ctx, _window_id, (unsigned char*)buffer, data_size, 0, 0, width, height);
            } else {
                int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
                cairo_surface_t* buffer_surface = cairo_image_surface_create_for_data((unsigned char*)buffer, CAIRO_FORMAT_ARGB32, width, height, stride);

                cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, rect.width, rect.height);
                cairo_t* cr = cairo_create(surface);

                cairo_set_source_surface(cr, buffer_surface, -rect.x, -rect.y);
                cairo_rectangle(cr, 0, 0, rect.width, rect.height);
                cairo_fill(cr);

                size_t data_size = rect.width * rect.height * 4;
                unsigned char* data = cairo_image_surface_get_data(surface);
                revyv_window_update(_revyv_ctx, _window_id, data, data_size, rect.x, rect.y, rect.width, rect.height);

                cairo_destroy(cr);
                cairo_surface_destroy(buffer_surface);
                cairo_surface_destroy(surface);
            }
        }
    }
}
