#ifndef BROWSERCLIENT_H
#define BROWSERCLIENT_H

#include "include/cef_client.h"
#include "render_handler.h"

class BrowserClient : public CefClient {
public:
    explicit BrowserClient(RenderHandler* renderHandler);

    CefRefPtr<CefRenderHandler> GetRenderHandler() override;

private:
    CefRefPtr<CefRenderHandler> _render_handler;

    IMPLEMENT_REFCOUNTING(BrowserClient);
};

#endif
