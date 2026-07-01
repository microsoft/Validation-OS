#include "AppWindow.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                      LPWSTR /*lpCmdLine*/, int nCmdShow)
{
    AppWindow app(hInstance);
    if (!app.Create(nCmdShow))
        return 1;

    return app.RunMessageLoop();
}
