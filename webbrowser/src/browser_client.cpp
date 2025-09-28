#include "browser_client.h"

BrowserClient::BrowserClient(RenderHandler* renderHandler)
    : _render_handler(renderHandler)
{
}

CefRefPtr<CefRenderHandler> BrowserClient::GetRenderHandler()
{
    return _render_handler;
}