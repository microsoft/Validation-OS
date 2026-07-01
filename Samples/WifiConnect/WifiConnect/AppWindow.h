#pragma once
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include "WifiManager.h"
#include "WifiNetwork.h"

// Owns the main application window and all child controls.
// Delegates all WLAN operations to WifiManager.
class AppWindow
{
public:
    explicit AppWindow(HINSTANCE hInstance);
    ~AppWindow() = default;

    AppWindow(const AppWindow&)            = delete;
    AppWindow& operator=(const AppWindow&) = delete;

    bool Create(int nCmdShow);
    int  RunMessageLoop();

private:
    // Window procedure trampoline
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    // Event handlers
    void OnCreate(HWND hwnd);
    void OnRefresh();
    void OnConnect();
    void OnDisconnect();
    void OnSelectionChanged();
    void OnTimer();

    // Helpers
    void         PopulateList(const std::vector<WifiNetwork>& networks);
    void         SetStatus(const std::wstring& text);
    WifiNetwork* SelectedNetwork();
    void         ShowPasswordField(bool visible);

    // Handles
    HINSTANCE m_hInstance          = nullptr;
    HWND      m_hwnd               = nullptr;
    HWND      m_hList              = nullptr;
    HWND      m_hEditPassword      = nullptr;
    HWND      m_hLabelPassword     = nullptr;
    HWND      m_hBtnConnect        = nullptr;
    HWND      m_hBtnDisconnect     = nullptr;
    HWND      m_hBtnRefresh        = nullptr;
    HWND      m_hStatus            = nullptr;

    WifiManager              m_wifi;
    std::vector<WifiNetwork> m_networks;

    // Control IDs
    static constexpr UINT_PTR IDC_LIST        = 101;
    static constexpr UINT_PTR IDC_PASSWORD    = 102;
    static constexpr UINT_PTR IDC_CONNECT     = 103;
    static constexpr UINT_PTR IDC_DISCONNECT  = 104;
    static constexpr UINT_PTR IDC_REFRESH     = 105;
    static constexpr UINT_PTR TIMER_STATUS    = 1;

    static constexpr int kWidth  = 500;
    static constexpr int kHeight = 390;
};
