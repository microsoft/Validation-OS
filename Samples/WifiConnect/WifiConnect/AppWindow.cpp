#include "AppWindow.h"
#include <string>
#pragma comment(lib, "comctl32.lib")

static constexpr wchar_t kClassName[] = L"WifiConnectWindow";

// ── Construction ───────────────────────────────────────────────

AppWindow::AppWindow(HINSTANCE hInstance)
    : m_hInstance(hInstance)
{}

bool AppWindow::Create(int nCmdShow)
{
    INITCOMMONCONTROLSEX icex{ sizeof(icex), ICC_LISTVIEW_CLASSES };
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = m_hInstance;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);

    // Fixed-size window — no resize/maximize needed for a simple connect dialog
    const DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
    m_hwnd = CreateWindowExW(0, kClassName, L"WiFi Connect",
                             style,
                             CW_USEDEFAULT, CW_USEDEFAULT, kWidth, kHeight,
                             nullptr, nullptr, m_hInstance, this);
    if (!m_hwnd)
        return false;

    if (!m_wifi.Initialize())
    {
        MessageBoxW(nullptr, L"No WiFi adapter found.", L"WiFi Connect", MB_ICONERROR);
        return false;
    }

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    OnRefresh();
    return true;
}

int AppWindow::RunMessageLoop()
{
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

// ── Window procedure ──────────────────────────────────────────────────────────

/*static*/ LRESULT CALLBACK AppWindow::WndProc(HWND hwnd, UINT msg,
                                                WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CREATE)
    {
        auto* pThis = reinterpret_cast<AppWindow*>(
            reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->OnCreate(hwnd);
    }

    auto* pThis = reinterpret_cast<AppWindow*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    return pThis ? pThis->HandleMessage(msg, wParam, lParam)
                 : DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT AppWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_REFRESH:    OnRefresh();    return 0;
        case IDC_CONNECT:    OnConnect();    return 0;
        case IDC_DISCONNECT: OnDisconnect(); return 0;
        }
        break;

    case WM_NOTIFY:
        if (reinterpret_cast<LPNMHDR>(lParam)->idFrom == IDC_LIST &&
            reinterpret_cast<LPNMHDR>(lParam)->code   == LVN_ITEMCHANGED)
        {
            OnSelectionChanged();
        }
        break;

    case WM_TIMER:
        if (wParam == TIMER_STATUS)
            OnTimer();
        return 0;

    case WM_DESTROY:
        KillTimer(m_hwnd, TIMER_STATUS);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(m_hwnd, msg, wParam, lParam);
}

// ── Window creation ───────────────────────────────────────────────────────────

void AppWindow::OnCreate(HWND hwnd)
{
    m_hwnd = hwnd;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    const int W = rc.right;

    // Network list (takes most of the window)
    m_hList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                              WS_CHILD | WS_VISIBLE |
                              LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                              0, 0, W, 255,
                              hwnd, reinterpret_cast<HMENU>(IDC_LIST),
                              m_hInstance, nullptr);

    struct Col { const wchar_t* name; int cx; };
    const Col cols[] = { {L"SSID", 210}, {L"Signal", 75},
                         {L"Security", 100}, {L"Status", 90} };
    for (int i = 0; i < 4; ++i)
    {
        LVCOLUMNW lvc{ LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM };
        lvc.iSubItem = i;
        lvc.cx       = cols[i].cx;
        lvc.pszText  = const_cast<LPWSTR>(cols[i].name);
        ListView_InsertColumn(m_hList, i, &lvc);
    }
    ListView_SetExtendedListViewStyle(m_hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Password row (hidden until a secured network is selected)
    m_hLabelPassword = CreateWindowW(L"STATIC", L"Password:",
                                     WS_CHILD | SS_SIMPLE,
                                     10, 264, 80, 22,
                                     hwnd, nullptr, m_hInstance, nullptr);

    m_hEditPassword = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                      WS_CHILD | ES_PASSWORD | ES_AUTOHSCROLL,
                                      94, 262, W - 104, 24,
                                      hwnd,
                                      reinterpret_cast<HMENU>(IDC_PASSWORD),
                                      m_hInstance, nullptr);

    // Action buttons
    auto makeBtn = [&](const wchar_t* label, int x, UINT_PTR id) -> HWND {
        return CreateWindowW(L"BUTTON", label,
                             WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                             x, 296, 100, 28,
                             hwnd, reinterpret_cast<HMENU>(id),
                             m_hInstance, nullptr);
    };
    m_hBtnRefresh    = makeBtn(L"Refresh",     10,  IDC_REFRESH);
    m_hBtnConnect    = makeBtn(L"Connect",    120,  IDC_CONNECT);
    m_hBtnDisconnect = makeBtn(L"Disconnect", 230,  IDC_DISCONNECT);

    // Status bar
    m_hStatus = CreateWindowW(L"STATIC", L"",
                              WS_CHILD | WS_VISIBLE | SS_SUNKEN,
                              0, 334, W, 22,
                              hwnd, nullptr, m_hInstance, nullptr);

    // Poll connection state every 3 s to keep the status bar current
    SetTimer(hwnd, TIMER_STATUS, 3000, nullptr);
}

// ── Event handlers ────────────────────────────────────────────────────────────

void AppWindow::OnRefresh()
{
    SetStatus(L"Scanning...");
    m_networks = m_wifi.Scan();
    PopulateList(m_networks);

    const auto connected = m_wifi.GetConnectedSsid();
    SetStatus(connected.empty() ? L"Not connected"
                                : L"Connected: " + connected);
}

void AppWindow::OnConnect()
{
    WifiNetwork* net = SelectedNetwork();
    if (!net)
    {
        MessageBoxW(m_hwnd, L"Select a network first.", L"WiFi Connect",
                    MB_ICONINFORMATION);
        return;
    }

    std::wstring password;
    if (net->isSecured)
    {
        wchar_t buf[256]{};
        GetWindowTextW(m_hEditPassword, buf, 256);
        password = buf;
        if (password.empty())
        {
            MessageBoxW(m_hwnd, L"Enter the password for this network.",
                        L"WiFi Connect", MB_ICONINFORMATION);
            return;
        }
    }

    SetStatus(L"Connecting to " + net->ssid + L"...");
    EnableWindow(m_hBtnConnect, FALSE);

    if (m_wifi.Connect(net->ssid, password))
    {
        SetWindowTextW(m_hEditPassword, L"");
        SetStatus(L"Connection request sent - refreshing in 3 s...");
    }
    else
    {
        SetStatus(L"Connection failed for: " + net->ssid);
    }

    EnableWindow(m_hBtnConnect, TRUE);
}

void AppWindow::OnDisconnect()
{
    m_wifi.Disconnect();
    SetStatus(L"Disconnected");
    OnRefresh();
}

void AppWindow::OnSelectionChanged()
{
    const WifiNetwork* net = SelectedNetwork();
    ShowPasswordField(net && net->isSecured);
}

void AppWindow::OnTimer()
{
    const auto connected = m_wifi.GetConnectedSsid();
    SetStatus(connected.empty() ? L"Not connected"
                                : L"Connected: " + connected);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void AppWindow::PopulateList(const std::vector<WifiNetwork>& networks)
{
    ListView_DeleteAllItems(m_hList);

    for (int i = 0; i < static_cast<int>(networks.size()); ++i)
    {
        const auto& n = networks[i];

        LVITEMW lvi{ LVIF_TEXT };
        lvi.iItem   = i;
        lvi.pszText = const_cast<LPWSTR>(n.ssid.c_str());
        ListView_InsertItem(m_hList, &lvi);

        const auto sig = std::to_wstring(n.signalQuality) + L"%";
        ListView_SetItemText(m_hList, i, 1,
                             const_cast<LPWSTR>(sig.c_str()));
        ListView_SetItemText(m_hList, i, 2,
                             const_cast<LPWSTR>(n.isSecured ? L"Secured" : L"Open"));
        ListView_SetItemText(m_hList, i, 3,
                             const_cast<LPWSTR>(n.isConnected ? L"\u2714 Connected" : L""));
    }
}

void AppWindow::SetStatus(const std::wstring& text)
{
    SetWindowTextW(m_hStatus, text.c_str());
}

WifiNetwork* AppWindow::SelectedNetwork()
{
    const int idx = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);
    if (idx < 0 || idx >= static_cast<int>(m_networks.size()))
        return nullptr;
    return &m_networks[idx];
}

void AppWindow::ShowPasswordField(bool visible)
{
    const int cmd = visible ? SW_SHOW : SW_HIDE;
    ShowWindow(m_hLabelPassword, cmd);
    ShowWindow(m_hEditPassword,  cmd);
    if (!visible)
        SetWindowTextW(m_hEditPassword, L"");
}
