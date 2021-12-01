#pragma once
#include "include/cef_app.h"

// Implement application-level callbacks for the render process.
class SimpleAppRender : public CefApp, public CefRenderProcessHandler
{
public:
    SimpleAppRender();

    // CefApp methods:
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
    {
        return this;
    }

    // CefRenderProcessHandler methods:
    virtual void OnWebKitInitialized() override;
    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;
    

private:
    // Include the default reference counting implementation.
IMPLEMENT_REFCOUNTING(SimpleAppRender);
};
