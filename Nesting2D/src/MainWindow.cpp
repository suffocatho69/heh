#include "MainWindow.h"
#include "resource.h"
#include "DXFReader.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <chrono>

// Win32 Controls Identifiers
#define IDC_MENU_PLIK_NEW       2001
#define IDC_MENU_PLIK_OPEN      2002
#define IDC_MENU_PLIK_EXIT      2003
#define IDC_MENU_POMOC_ABOUT    2004

#define IDC_TOOLBAR             3001
#define IDC_TB_BTN_NEW          3101
#define IDC_TB_BTN_OPEN         3102
#define IDC_TB_BTN_PRINT        3103
#define IDC_TB_BTN_DXF          3104
#define IDC_TB_BTN_SHEETS       3105
#define IDC_TB_BTN_SETTINGS     3106
#define IDC_TB_BTN_DELETE       3107
#define IDC_TB_BTN_NESTING      3108
#define IDC_TB_BTN_NC           3109
#define IDC_TB_BTN_PDF          3110
#define IDC_TB_BTN_ZIP          3111

#define IDC_LIST_DB             4001
#define IDC_EDIT_SEARCH         4002
#define IDC_BTN_SEARCH_PENCIL   4003
#define IDC_BTN_SEARCH_PLAY     4004
#define IDC_BTN_RESET           4005
#define IDC_BTN_PLUSMINUS       4006
#define IDC_BTN_SCALE           4007

#define IDC_LIST_COMP           5001
#define IDC_BTN_ADD_COMP        5002
#define IDC_BTN_REMOVE_COMP     5003
#define IDC_BTN_MOVE_UP         5004
#define IDC_BTN_MOVE_DOWN       5005

#define IDC_CANVAS              6001

#define IDC_EDIT_PLATEW         7001
#define IDC_EDIT_PLATEH         7002
#define IDC_EDIT_MARGIN         7003
#define IDC_EDIT_SPACING        7004
#define IDC_CHK_ROT0            7005
#define IDC_CHK_ROT90           7006
#define IDC_CHK_ROT180          7007
#define IDC_CHK_ROT270          7008
#define IDC_COMBO_TOOL          7009
#define IDC_EDIT_FEED           7010
#define IDC_BTN_GENERATE_NESTING 7011

#define IDC_STATUSBAR           8001

// Global class name
const char g_szClassName[] = "Nesting2DMainWindowClass";

MainWindow::MainWindow()
    : m_hwnd(NULL), m_hInstance(NULL), m_hMenu(NULL), m_hToolbar(NULL),
      m_hGrpBazaDetali(NULL), m_hListViewDB(NULL), m_hEditSearch(NULL),
      m_hBtnSearchPencil(NULL), m_hBtnSearchPlay(NULL), m_hBtnReset(NULL),
      m_hBtnPlusMinus(NULL), m_hBtnScale(NULL), m_hGrpKomponenty(NULL),
      m_hListViewComp(NULL), m_hBtnAddComp(NULL), m_hBtnRemoveComp(NULL),
      m_hBtnMoveUp(NULL), m_hBtnMoveDown(NULL), m_hGrpPodgladPlyty(NULL),
      m_hCanvas(NULL), m_hGrpParametry(NULL), m_hEditPlateW(NULL),
      m_hEditMargin(NULL), m_hEditSpacing(NULL), m_hChkRot0(NULL), m_hChkRot90(NULL),
      m_hChkRot180(NULL), m_hChkRot270(NULL), m_hComboTool(NULL), m_hEditFeed(NULL),
      m_hBtnGenerateNesting(NULL), m_hStatusBar(NULL), m_activeSheetIndex(-1),
      m_lastNestingTime(0.0) {

    // Seed database items and components for "Nestingator3000" visual styling
    m_compManager.addComponent(Component("Bok.dxf", 600.0, 300.0, 2));
    m_compManager.addComponent(Component("Front.dxf", 400.0, 400.0, 8));
    m_compManager.addComponent(Component("Plecy.dxf", 800.0, 500.0, 1));
}

MainWindow::~MainWindow() {
}

bool MainWindow::Create(HINSTANCE hInstance, int nCmdShow) {
    m_hInstance = hInstance;

    WNDCLASSEX wc;
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MainWindow::WindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = m_hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); // Win95/98 Light Gray look
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Set up standard Menu Bar structure
    m_hMenu = CreateMenu();
    HMENU hMenuPlik = CreatePopupMenu();
    AppendMenu(hMenuPlik, MF_STRING, IDC_MENU_PLIK_NEW, "Nowy");
    AppendMenu(hMenuPlik, MF_STRING, IDC_MENU_PLIK_OPEN, "Otwórz...");
    AppendMenu(hMenuPlik, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuPlik, MF_STRING, IDC_MENU_PLIK_EXIT, "Wyjdź");
    AppendMenu(m_hMenu, MF_POPUP, (UINT_PTR)hMenuPlik, "Plik");

    HMENU hMenuEdycja = CreatePopupMenu();
    AppendMenu(hMenuEdycja, MF_STRING, 0, "Cofnij");
    AppendMenu(hMenuEdycja, MF_STRING, 0, "Wytnij");
    AppendMenu(hMenuEdycja, MF_STRING, 0, "Kopiuj");
    AppendMenu(hMenuEdycja, MF_STRING, 0, "Wklej");
    AppendMenu(m_hMenu, MF_POPUP, (UINT_PTR)hMenuEdycja, "Edycja");

    HMENU hMenuWidok = CreatePopupMenu();
    AppendMenu(hMenuWidok, MF_STRING, 0, "Paski narzędzi");
    AppendMenu(hMenuWidok, MF_STRING, 0, "Pasek stanu");
    AppendMenu(m_hMenu, MF_POPUP, (UINT_PTR)hMenuWidok, "Widok");

    HMENU hMenuNarzedzia = CreatePopupMenu();
    AppendMenu(hMenuNarzedzia, MF_STRING, IDC_TB_BTN_NESTING, "Generuj Nesting");
    AppendMenu(hMenuNarzedzia, MF_STRING, IDC_TB_BTN_NC, "Generuj Kod NC");
    AppendMenu(m_hMenu, MF_POPUP, (UINT_PTR)hMenuNarzedzia, "Narzędzia");

    HMENU hMenuBiblioteka = CreatePopupMenu();
    AppendMenu(hMenuBiblioteka, MF_STRING, 0, "Zarządzaj biblioteką...");
    AppendMenu(m_hMenu, MF_POPUP, (UINT_PTR)hMenuBiblioteka, "Biblioteka");

    HMENU hMenuPomoc = CreatePopupMenu();
    AppendMenu(hMenuPomoc, MF_STRING, IDC_MENU_POMOC_ABOUT, "O programie...");
    AppendMenu(m_hMenu, MF_POPUP, (UINT_PTR)hMenuPomoc, "Pomoc");

    // Nestingator3000 main frame dimension
    m_hwnd = CreateWindowEx(
        0,
        g_szClassName,
        "Nestingator3000",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 780,
        NULL, m_hMenu, m_hInstance, this
    );

    if (m_hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    return true;
}

int MainWindow::Run() {
    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return static_cast<int>(Msg.wParam);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = NULL;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (MainWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        return pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
    } else {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT MainWindow::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            InitControls(hwnd);
            RefreshListView();
            break;
        }
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            ResizeControls(width, height);
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            switch (wmId) {
                case IDC_MENU_PLIK_NEW:
                    m_compManager.clearAllComponents();
                    m_sheets.clear();
                    m_activeSheetIndex = -1;
                    RefreshListView();
                    InvalidateRect(m_hCanvas, NULL, TRUE);
                    break;
                case IDC_MENU_PLIK_OPEN:
                case IDC_TB_BTN_OPEN:
                case IDC_TB_BTN_DXF:
                    OnImportDXF();
                    break;
                case IDC_MENU_PLIK_EXIT:
                    DestroyWindow(hwnd);
                    break;
                case IDC_MENU_POMOC_ABOUT:
                    MessageBox(hwnd, "Nestingator3000 v1.2.0\nSystem optymalizacji rozkroju detali 2D\nOrientica.pl", "O programie", MB_OK | MB_ICONINFORMATION);
                    break;
                case IDC_BTN_ADD_COMP:
                case IDC_TB_BTN_NEW:
                    OnAddComponent();
                    break;
                case IDC_BTN_REMOVE_COMP:
                case IDC_TB_BTN_DELETE:
                    OnRemoveComponent();
                    break;
                case IDC_BTN_GENERATE_NESTING:
                case IDC_TB_BTN_NESTING:
                    OnRunNesting();
                    break;
                case IDC_TB_BTN_NC:
                    OnExportGCode();
                    break;
                case IDC_BTN_RESET: {
                    SetWindowText(m_hEditSearch, "");
                    break;
                }
            }
            break;
        }
        case WM_DRAWITEM: {
            DRAWITEMSTRUCT* pDIS = (DRAWITEMSTRUCT*)lParam;
            if (pDIS->CtlID == IDC_CANVAS) {
                OnPaintCanvas(pDIS->hwndItem, pDIS->hDC);
                return TRUE;
            } else if (pDIS->CtlID == IDC_BTN_GENERATE_NESTING) {
                // Style GENERUJ NESTING to look exactly like the reference image (Deep Blue background, White text)
                HDC hdc = pDIS->hDC;
                RECT rc = pDIS->rcItem;

                // Draw deep blue button face
                HBRUSH hBrush = CreateSolidBrush(RGB(51, 102, 187));
                FillRect(hdc, &rc, hBrush);
                DeleteObject(hBrush);

                // Draw standard 3D double border
                HPEN hPenLight = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                HPEN hPenDark = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));

                if (pDIS->itemState & ODS_SELECTED) {
                    SelectObject(hdc, hPenDark);
                    MoveToEx(hdc, rc.left, rc.top, NULL);
                    LineTo(hdc, rc.right, rc.top);
                    LineTo(hdc, rc.right, rc.bottom);
                    LineTo(hdc, rc.left, rc.bottom);
                    LineTo(hdc, rc.left, rc.top);
                } else {
                    SelectObject(hdc, hPenLight);
                    MoveToEx(hdc, rc.left, rc.bottom, NULL);
                    LineTo(hdc, rc.left, rc.top);
                    LineTo(hdc, rc.right, rc.top);

                    SelectObject(hdc, hPenDark);
                    LineTo(hdc, rc.right, rc.bottom);
                    LineTo(hdc, rc.left, rc.bottom);
                }

                DeleteObject(hPenLight);
                DeleteObject(hPenDark);

                // Set text styling
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(255, 255, 255));
                HFONT hFontBtn = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");
                HGDIOBJ oldFont = SelectObject(hdc, hFontBtn);

                DrawText(hdc, "GENERUJ NESTING", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                SelectObject(hdc, oldFont);
                DeleteObject(hFontBtn);
                return TRUE;
            }
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MainWindow::InitControls(HWND hwnd) {
    // Elegant Retro Sans-Serif Fonts
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");
    HFONT hFontBold = CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

    // 1. Create Toolbar (Stylized flat panel buttons matching Nestingator3000)
    m_hToolbar = CreateWindowEx(0, "STATIC", "",
        WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        0, 0, 1100, 50, hwnd, (HMENU)IDC_TOOLBAR, m_hInstance, NULL);

    // Create Retro toolbar pushbuttons
    auto CreateToolbarBtn = [&](const std::string& txt, int id, int x, int w) {
        HWND hBtn = CreateWindowEx(0, "BUTTON", txt.c_str(),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            x, 8, w, 34, hwnd, (HMENU)(INT_PTR)id, m_hInstance, NULL);
        SendMessage(hBtn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
    };

    CreateToolbarBtn("New", IDC_TB_BTN_NEW, 10, 45);
    CreateToolbarBtn("Open", IDC_TB_BTN_OPEN, 60, 50);
    CreateToolbarBtn("Print", IDC_TB_BTN_PRINT, 115, 50);
    CreateToolbarBtn("DXF", IDC_TB_BTN_DXF, 170, 45);
    CreateToolbarBtn("Płyty", IDC_TB_BTN_SHEETS, 220, 50);
    CreateToolbarBtn("Settings", IDC_TB_BTN_SETTINGS, 275, 70);
    CreateToolbarBtn("Delete", IDC_TB_BTN_DELETE, 350, 60);
    CreateToolbarBtn("Nesting", IDC_TB_BTN_NESTING, 420, 75);
    CreateToolbarBtn("NC", IDC_TB_BTN_NC, 505, 45);
    CreateToolbarBtn("PDF", IDC_TB_BTN_PDF, 555, 45);
    CreateToolbarBtn("ZIP", IDC_TB_BTN_ZIP, 605, 45);

    // 2. LEFT-HAND PANELS (DB and Components)
    m_hGrpBazaDetali = CreateWindowEx(0, "BUTTON", "BAZA DETALI",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        10, 60, 450, 310, hwnd, NULL, m_hInstance, NULL);
    SendMessage(m_hGrpBazaDetali, WM_SETFONT, (WPARAM)hFontBold, TRUE);

    // Baza Detali Folder List with custom folders: Elementy, Meble, Fronty, Własne
    m_hListViewDB = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL,
        20, 85, 430, 185, hwnd, (HMENU)IDC_LIST_DB, m_hInstance, NULL);
    SendMessage(m_hListViewDB, WM_SETFONT, (WPARAM)hFont, TRUE);
    ListView_SetExtendedListViewStyle(m_hListViewDB, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.cx = 400;
    lvc.pszText = (LPSTR)"Kategoria";
    ListView_InsertColumn(m_hListViewDB, 0, &lvc);

    // Seed visual folders
    const char* folders[] = { "📁 Elementy", "📁 Meble", "📁 Fronty", "📁 Własne" };
    for (int i = 0; i < 4; ++i) {
        LVITEM lvi;
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.pszText = (LPSTR)folders[i];
        ListView_InsertItem(m_hListViewDB, &lvi);
        ListView_SetCheckState(m_hListViewDB, i, TRUE);
    }

    // Search Box and buttons
    HWND hLabelSzukaj = CreateWindowEx(0, "STATIC", "Szukaj:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 283, 50, 20, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hLabelSzukaj, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hEditSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        75, 280, 210, 22, hwnd, (HMENU)IDC_EDIT_SEARCH, m_hInstance, NULL);
    SendMessage(m_hEditSearch, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hBtnSearchPencil = CreateWindowEx(0, "BUTTON", "🖍",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        290, 280, 25, 22, hwnd, (HMENU)IDC_BTN_SEARCH_PENCIL, m_hInstance, NULL);
    SendMessage(m_hBtnSearchPencil, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hBtnSearchPlay = CreateWindowEx(0, "BUTTON", "▶",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        320, 280, 25, 22, hwnd, (HMENU)IDC_BTN_SEARCH_PLAY, m_hInstance, NULL);
    SendMessage(m_hBtnSearchPlay, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hBtnReset = CreateWindowEx(0, "BUTTON", "[ Reset",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        350, 280, 45, 22, hwnd, (HMENU)IDC_BTN_RESET, m_hInstance, NULL);
    SendMessage(m_hBtnReset, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hBtnPlusMinus = CreateWindowEx(0, "BUTTON", "+-]",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        400, 280, 25, 22, hwnd, (HMENU)IDC_BTN_PLUSMINUS, m_hInstance, NULL);
    SendMessage(m_hBtnPlusMinus, WM_SETFONT, (WPARAM)hFont, TRUE);

    // KOMPONENTY PANEL
    m_hGrpKomponenty = CreateWindowEx(0, "BUTTON", "Komponenty",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        10, 380, 450, 315, hwnd, NULL, m_hInstance, NULL);
    SendMessage(m_hGrpKomponenty, WM_SETFONT, (WPARAM)hFontBold, TRUE);

    m_hListViewComp = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        20, 405, 430, 230, hwnd, (HMENU)IDC_LIST_COMP, m_hInstance, NULL);
    SendMessage(m_hListViewComp, WM_SETFONT, (WPARAM)hFont, TRUE);
    ListView_SetExtendedListViewStyle(m_hListViewComp, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES);

    // Add component list columns
    LVCOLUMN lvcC;
    lvcC.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvcC.cx = 200;
    lvcC.pszText = (LPSTR)"Nazwa";
    ListView_InsertColumn(m_hListViewComp, 0, &lvcC);

    lvcC.cx = 100;
    lvcC.pszText = (LPSTR)"Szer (X)";
    ListView_InsertColumn(m_hListViewComp, 1, &lvcC);

    lvcC.cx = 100;
    lvcC.pszText = (LPSTR)"Wys (Y)";
    ListView_InsertColumn(m_hListViewComp, 2, &lvcC);

    lvcC.cx = 60;
    lvcC.pszText = (LPSTR)"Ilość";
    ListView_InsertColumn(m_hListViewComp, 3, &lvcC);

    // Component Control Buttons
    m_hBtnAddComp = CreateWindowEx(0, "BUTTON", "+ Dodaj]",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 645, 100, 30, hwnd, (HMENU)IDC_BTN_ADD_COMP, m_hInstance, NULL);
    SendMessage(m_hBtnAddComp, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hBtnRemoveComp = CreateWindowEx(0, "BUTTON", "+ Usuń]",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        130, 645, 100, 30, hwnd, (HMENU)IDC_BTN_REMOVE_COMP, m_hInstance, NULL);
    SendMessage(m_hBtnRemoveComp, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hBtnMoveUp = CreateWindowEx(0, "BUTTON", "▲",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        370, 645, 35, 30, hwnd, (HMENU)IDC_BTN_MOVE_UP, m_hInstance, NULL);
    SendMessage(m_hBtnMoveUp, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hBtnMoveDown = CreateWindowEx(0, "BUTTON", "▼",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        415, 645, 35, 30, hwnd, (HMENU)IDC_BTN_MOVE_DOWN, m_hInstance, NULL);
    SendMessage(m_hBtnMoveDown, WM_SETFONT, (WPARAM)hFont, TRUE);


    // 3. RIGHT-HAND PANELS (Visualization & Params)
    m_hGrpPodgladPlyty = CreateWindowEx(0, "BUTTON", "PODGLĄD PŁYTY",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        470, 60, 600, 380, hwnd, NULL, m_hInstance, NULL);
    SendMessage(m_hGrpPodgladPlyty, WM_SETFONT, (WPARAM)hFontBold, TRUE);

    // Standard Retro Canvas GDI display
    m_hCanvas = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "",
        WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        485, 85, 570, 340, hwnd, (HMENU)IDC_CANVAS, m_hInstance, NULL);

    // Parametry Panel
    m_hGrpParametry = CreateWindowEx(0, "BUTTON", "Parametry",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        470, 450, 600, 245, hwnd, NULL, m_hInstance, NULL);
    SendMessage(m_hGrpParametry, WM_SETFONT, (WPARAM)hFontBold, TRUE);

    // Left Columns inside Parametry
    HWND hLabelPlyta = CreateWindowEx(0, "STATIC", "Płyta:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        485, 483, 70, 20, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hLabelPlyta, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hEditPlateW = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "3000 x 1500",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        560, 480, 140, 22, hwnd, (HMENU)IDC_EDIT_PLATEW, m_hInstance, NULL);
    SendMessage(m_hEditPlateW, WM_SETFONT, (WPARAM)hFont, TRUE);

    HWND hLabelMargin = CreateWindowEx(0, "STATIC", "Margines :",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        485, 513, 70, 20, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hLabelMargin, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hEditMargin = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "10 mm",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        560, 510, 140, 22, hwnd, (HMENU)IDC_EDIT_MARGIN, m_hInstance, NULL);
    SendMessage(m_hEditMargin, WM_SETFONT, (WPARAM)hFont, TRUE);

    HWND hLabelSpacing = CreateWindowEx(0, "STATIC", "Odstęp:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        485, 543, 70, 20, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hLabelSpacing, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hEditSpacing = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", ":5 mm",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        560, 540, 140, 22, hwnd, (HMENU)IDC_EDIT_SPACING, m_hInstance, NULL);
    SendMessage(m_hEditSpacing, WM_SETFONT, (WPARAM)hFont, TRUE);

    HWND hLabelObrot = CreateWindowEx(0, "STATIC", "Obrót:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        485, 573, 70, 20, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hLabelObrot, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Rotation select checkboxes
    m_hChkRot0 = CreateWindowEx(0, "BUTTON", "0",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        560, 571, 35, 20, hwnd, (HMENU)IDC_CHK_ROT0, m_hInstance, NULL);
    SendMessage(m_hChkRot0, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hChkRot0, BM_SETCHECK, BST_CHECKED, 0);

    m_hChkRot90 = CreateWindowEx(0, "BUTTON", "90",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        600, 571, 40, 20, hwnd, (HMENU)IDC_CHK_ROT90, m_hInstance, NULL);
    SendMessage(m_hChkRot90, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hChkRot90, BM_SETCHECK, BST_CHECKED, 0);

    m_hChkRot180 = CreateWindowEx(0, "BUTTON", "180",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        645, 571, 45, 20, hwnd, (HMENU)IDC_CHK_ROT180, m_hInstance, NULL);
    SendMessage(m_hChkRot180, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hChkRot270 = CreateWindowEx(0, "BUTTON", "270",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        695, 571, 45, 20, hwnd, (HMENU)IDC_CHK_ROT270, m_hInstance, NULL);
    SendMessage(m_hChkRot270, WM_SETFONT, (WPARAM)hFont, TRUE);

    // NC Parameters (Right half of parameters)
    HWND hLabelNC = CreateWindowEx(0, "STATIC", "NC:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        730, 483, 80, 20, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hLabelNC, WM_SETFONT, (WPARAM)hFontBold, TRUE);

    HWND hLabelNarzedzie = CreateWindowEx(0, "STATIC", "Narzędzie:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        730, 513, 80, 20, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hLabelNarzedzie, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hComboTool = CreateWindowEx(0, "COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        820, 510, 160, 120, hwnd, (HMENU)IDC_COMBO_TOOL, m_hInstance, NULL);
    SendMessage(m_hComboTool, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hComboTool, CB_ADDSTRING, 0, (LPARAM)"Oscylacyjne");
    SendMessage(m_hComboTool, CB_ADDSTRING, 0, (LPARAM)"Frez dyskowy");
    SendMessage(m_hComboTool, CB_ADDSTRING, 0, (LPARAM)"Laser");
    SendMessage(m_hComboTool, CB_SETCURSEL, 0, 0);

    HWND hLabelPosuw = CreateWindowEx(0, "STATIC", "Posuw:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        730, 543, 80, 20, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hLabelPosuw, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hEditFeed = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "1200",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        820, 540, 160, 22, hwnd, (HMENU)IDC_EDIT_FEED, m_hInstance, NULL);
    SendMessage(m_hEditFeed, WM_SETFONT, (WPARAM)hFont, TRUE);

    // OWNER DRAW GENERATE BUTTON (Beautiful deep blue style)
    m_hBtnGenerateNesting = CreateWindowEx(0, "BUTTON", "GENERUJ NESTING",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        730, 595, 250, 40, hwnd, (HMENU)IDC_BTN_GENERATE_NESTING, m_hInstance, NULL);

    // 4. BOTTOM STATUS BAR
    m_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, "",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd, (HMENU)IDC_STATUSBAR, m_hInstance, NULL);

    int sbParts[] = { 200, 400, 700, 900, -1 };
    SendMessage(m_hStatusBar, SB_SETPARTS, 5, (LPARAM)sbParts);
    SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)"18 detali");
    SendMessage(m_hStatusBar, SB_SETTEXT, 1, (LPARAM)"2 płyty");
    SendMessage(m_hStatusBar, SB_SETTEXT, 2, (LPARAM)"Wykorzystanie 92.41%");
    SendMessage(m_hStatusBar, SB_SETTEXT, 3, (LPARAM)"Czas 1.82 s");
    SendMessage(m_hStatusBar, SB_SETTEXT, 4, (LPARAM)"5 wątków");
}

void MainWindow::ResizeControls(int width, int height) {
    if (m_hStatusBar) {
        SendMessage(m_hStatusBar, WM_SIZE, 0, 0);

        // Compute standard proportional layout spaces
        int rightWidth = width - 480;
        int bottomHeight = height - 160;
        if (rightWidth < 200) rightWidth = 200;
        if (bottomHeight < 150) bottomHeight = 150;

        // Reposition left panel groupboxes and controls
        MoveWindow(m_hGrpBazaDetali, 10, 60, 450, 290, TRUE);
        MoveWindow(m_hListViewDB, 20, 85, 430, 185, TRUE);
        MoveWindow(m_hEditSearch, 75, 280, 210, 22, TRUE);
        MoveWindow(m_hBtnSearchPencil, 290, 280, 25, 22, TRUE);
        MoveWindow(m_hBtnSearchPlay, 320, 280, 25, 22, TRUE);
        MoveWindow(m_hBtnReset, 350, 280, 45, 22, TRUE);
        MoveWindow(m_hBtnPlusMinus, 400, 280, 25, 22, TRUE);

        MoveWindow(m_hGrpKomponenty, 10, 360, 450, height - 420, TRUE);
        MoveWindow(m_hListViewComp, 20, 385, 430, height - 495, TRUE);
        MoveWindow(m_hBtnAddComp, 20, height - 100, 100, 30, TRUE);
        MoveWindow(m_hBtnRemoveComp, 130, height - 100, 100, 30, TRUE);
        MoveWindow(m_hBtnMoveUp, 370, height - 100, 35, 30, TRUE);
        MoveWindow(m_hBtnMoveDown, 415, height - 100, 35, 30, TRUE);

        // Reposition right panel groupboxes and controls
        MoveWindow(m_hGrpPodgladPlyty, 470, 60, rightWidth - 10, height - 360, TRUE);
        MoveWindow(m_hCanvas, 485, 85, rightWidth - 40, height - 405, TRUE);

        MoveWindow(m_hGrpParametry, 470, height - 290, rightWidth - 10, 230, TRUE);
    }
}

void MainWindow::RefreshListView() {
    ListView_DeleteAllItems(m_hListViewComp);

    const auto& comps = m_compManager.getComponents();
    for (size_t i = 0; i < comps.size(); ++i) {
        const auto& c = comps[i];

        LVITEM lvi;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = static_cast<int>(i);
        lvi.iSubItem = 0;
        lvi.pszText = (LPSTR)c.name.c_str();
        lvi.lParam = (LPARAM)i;
        ListView_InsertItem(m_hListViewComp, &lvi);
        ListView_SetCheckState(m_hListViewComp, i, TRUE);

        std::string wStr = std::to_string((int)c.width);
        ListView_SetItemText(m_hListViewComp, static_cast<int>(i), 1, (LPSTR)wStr.c_str());

        std::string hStr = std::to_string((int)c.height);
        ListView_SetItemText(m_hListViewComp, static_cast<int>(i), 2, (LPSTR)hStr.c_str());

        std::string qStr = "x" + std::to_string(c.quantity);
        ListView_SetItemText(m_hListViewComp, static_cast<int>(i), 3, (LPSTR)qStr.c_str());
    }
}

void MainWindow::OnAddComponent() {
    Component manual("Nowy_Detal.dxf", 350.0, 350.0, 3);
    m_compManager.addComponent(manual);
    RefreshListView();
}

void MainWindow::OnRemoveComponent() {
    int idx = ListView_GetNextItem(m_hListViewComp, -1, LVNI_SELECTED);
    if (idx != -1) {
        m_compManager.removeComponent(idx);
        RefreshListView();
    } else {
        MessageBox(m_hwnd, "Wybierz najpierw element z listy do usunięcia!", "Informacja", MB_OK | MB_ICONINFORMATION);
    }
}

void MainWindow::OnImportDXF() {
    OPENFILENAME ofn;
    char szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Pliki DXF (*.dxf)\0*.dxf\0Wszystkie Pliki (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        Component dxfComp;
        if (DXFReader::loadDXF(szFile, dxfComp)) {
            dxfComp.quantity = 5; // Default quantity
            m_compManager.addComponent(dxfComp);
            RefreshListView();
            MessageBox(m_hwnd, "Plik DXF zaimportowany pomyślnie!", "Sukces", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(m_hwnd, "Nie udało się zaimportować pliku DXF. Plik może być uszkodzony lub pusty.", "Błąd", MB_OK | MB_ICONERROR);
        }
    }
}

void MainWindow::OnRunNesting() {
    char buf[128];

    // Parse input fields securely
    GetWindowText(m_hEditPlateW, buf, sizeof(buf));
    std::string plateStr(buf);
    size_t xPos = plateStr.find('x');
    if (xPos != std::string::npos) {
        m_nestingParams.sheetWidth = atof(plateStr.substr(0, xPos).c_str());
        m_nestingParams.sheetHeight = atof(plateStr.substr(xPos + 1).c_str());
    } else {
        m_nestingParams.sheetWidth = 3000.0;
        m_nestingParams.sheetHeight = 1500.0;
    }

    if (m_nestingParams.sheetWidth <= 0) m_nestingParams.sheetWidth = 3000.0;
    if (m_nestingParams.sheetHeight <= 0) m_nestingParams.sheetHeight = 1500.0;

    GetWindowText(m_hEditMargin, buf, sizeof(buf));
    m_nestingParams.margin = atof(buf);
    if (m_nestingParams.margin <= 0) m_nestingParams.margin = 10.0;

    GetWindowText(m_hEditSpacing, buf, sizeof(buf));
    std::string spacingStr(buf);
    if (spacingStr.front() == ':') {
        m_nestingParams.spacing = atof(spacingStr.substr(1).c_str());
    } else {
        m_nestingParams.spacing = atof(spacingStr.c_str());
    }
    if (m_nestingParams.spacing <= 0) m_nestingParams.spacing = 5.0;

    // Custom rotations
    m_nestingParams.allowRot0 = (SendMessage(m_hChkRot0, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_nestingParams.allowRot90 = (SendMessage(m_hChkRot90, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_nestingParams.allowRot180 = (SendMessage(m_hChkRot180, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_nestingParams.allowRot270 = (SendMessage(m_hChkRot270, BM_GETCHECK, 0, 0) == BST_CHECKED);

    // Read NC parameters
    GetWindowText(m_hEditFeed, buf, sizeof(buf));
    m_ncParams.cuttingFeed = atof(buf);
    if (m_ncParams.cuttingFeed <= 0) m_ncParams.cuttingFeed = 1200.0;

    // Run nesting logic & time performance
    auto start = std::chrono::high_resolution_clock::now();
    m_sheets = NestingEngine::performNesting(m_compManager.getComponents(), m_nestingParams);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    m_lastNestingTime = diff.count();

    if (m_sheets.empty()) {
        MessageBox(m_hwnd, "Brak elementów do ułożenia lub elementy za duże!", "Nesting Błąd", MB_OK | MB_ICONWARNING);
        return;
    }

    m_activeSheetIndex = 0;

    // Calculate total original parts count
    int totalOriginal = 0;
    for (const auto& c : m_compManager.getComponents()) {
        totalOriginal += c.quantity;
    }
    NestingStats stats = ComponentManager::calculateStats(m_sheets, totalOriginal);

    // Dynamically update bottom status bar fields
    std::string itemText = std::to_string(totalOriginal) + " detali";
    SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)itemText.c_str());

    std::string sheetText = std::to_string(stats.totalSheets) + " płyty";
    SendMessage(m_hStatusBar, SB_SETTEXT, 1, (LPARAM)sheetText.c_str());

    std::stringstream ssUtil;
    ssUtil << std::fixed << std::setprecision(2);
    ssUtil << "Wykorzystanie " << stats.materialUtilization << "%";
    SendMessage(m_hStatusBar, SB_SETTEXT, 2, (LPARAM)ssUtil.str().c_str());

    std::stringstream ssTime;
    ssTime << std::fixed << std::setprecision(2);
    ssTime << "Czas " << m_lastNestingTime << " s";
    SendMessage(m_hStatusBar, SB_SETTEXT, 3, (LPARAM)ssTime.str().c_str());

    SendMessage(m_hStatusBar, SB_SETTEXT, 4, (LPARAM)"5 wątków");

    // Redraw
    InvalidateRect(m_hCanvas, NULL, TRUE);
}

void MainWindow::OnExportGCode() {
    if (m_sheets.empty()) return;

    OPENFILENAME ofn;
    char szFile[260] = "program_rozkroju.nc";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Pliki G-code (*.nc;*.gcode)\0*.nc;*.gcode\0Wszystkie Pliki (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE) {
        std::string gcode = NCGenerator::generateGCode(m_sheets, m_ncParams);

        std::ofstream outfile(szFile);
        if (outfile.is_open()) {
            outfile << gcode;
            MessageBox(m_hwnd, "Plik G-kod wygenerowany i zapisany pomyślnie!", "Eksport CNC", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(m_hwnd, "Nie udało się zapisać pliku. Brak uprawnień do katalogu.", "Błąd zapisu", MB_OK | MB_ICONERROR);
        }
    }
}

void MainWindow::OnPaintCanvas(HWND hwnd, HDC hdc) {
    RECT rect;
    GetClientRect(hwnd, &rect);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

    // Use SaveDC to prevent resource leaks during drawing
    int saveId = SaveDC(memDC);

    // Canvas background matching the dark tech grid style of Nestingator3000 (Dark Slate Gray)
    HBRUSH bgBrush = CreateSolidBrush(RGB(40, 40, 40));
    FillRect(memDC, &rect, bgBrush);
    DeleteObject(bgBrush);

    // If sheets exist, draw the selected layout
    if (m_activeSheetIndex >= 0 && m_activeSheetIndex < static_cast<int>(m_sheets.size())) {
        const auto& sheet = m_sheets[m_activeSheetIndex];

        double pad = 15.0;
        double canvasW = rect.right - 2 * pad;
        double canvasH = rect.bottom - 2 * pad;

        double scaleX = canvasW / sheet.width;
        double scaleY = canvasH / sheet.height;
        double scale = (scaleX < scaleY) ? scaleX : scaleY;

        double offX = pad + (canvasW - sheet.width * scale) / 2.0;
        double offY = pad + (canvasH - sheet.height * scale) / 2.0;

        // Draw outer board boundaries
        HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
        SelectObject(memDC, borderPen);

        HBRUSH sheetBrush = CreateSolidBrush(RGB(65, 65, 65));
        SelectObject(memDC, sheetBrush);

        Rectangle(memDC,
                  (int)offX,
                  (int)offY,
                  (int)(offX + sheet.width * scale),
                  (int)(offY + sheet.height * scale));

        DeleteObject(sheetBrush);
        DeleteObject(borderPen);

        // Grid lines (retro technical pattern)
        HPEN gridPen = CreatePen(PS_DOT, 1, RGB(80, 80, 80));
        SelectObject(memDC, gridPen);
        for (double gx = 100.0; gx < sheet.width; gx += 100.0) {
            MoveToEx(memDC, (int)(offX + gx * scale), (int)offY, NULL);
            LineTo(memDC, (int)(offX + gx * scale), (int)(offY + sheet.height * scale));
        }
        for (double gy = 100.0; gy < sheet.height; gy += 100.0) {
            MoveToEx(memDC, (int)offX, (int)(offY + gy * scale), NULL);
            LineTo(memDC, (int)(offX + sheet.width * scale), (int)(offY + gy * scale));
        }
        DeleteObject(gridPen);

        // Draw components in stylized colors (Red, Blue, Gray) as shown in the mockup image
        int count = 0;
        for (const auto& comp : sheet.placedComponents) {
            double ew = comp.getEffectiveWidth();
            double eh = comp.getEffectiveHeight();

            int sx = (int)(offX + comp.posX * scale);
            int sy = (int)(offY + comp.posY * scale);
            int sw = (int)(ew * scale);
            int sh = (int)(eh * scale);

            // Assign block colors exactly as in the mock image (Blue, Red, Gray)
            COLORREF blockColor = RGB(220, 220, 220); // standard gray
            if (count % 7 == 0) {
                blockColor = RGB(204, 30, 30); // deep red
            } else if (count % 5 == 0) {
                blockColor = RGB(20, 90, 210); // deep blue
            }

            HBRUSH compBrush = CreateSolidBrush(blockColor);
            HPEN compPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));

            SelectObject(memDC, compBrush);
            SelectObject(memDC, compPen);

            Rectangle(memDC, sx, sy, sx + sw, sy + sh);

            DeleteObject(compBrush);
            DeleteObject(compPen);

            // Geometry pathways inside component
            HPEN pathPen = CreatePen(PS_SOLID, 1, RGB(30, 30, 30));
            SelectObject(memDC, pathPen);

            for (const auto& geo : comp.geometry) {
                if (geo.type == GeoEntity::LINE) {
                    auto p1 = comp.localToGlobal(geo.x1, geo.y1);
                    auto p2 = comp.localToGlobal(geo.x2, geo.y2);

                    MoveToEx(memDC, (int)(offX + p1.first * scale), (int)(offY + p1.second * scale), NULL);
                    LineTo(memDC, (int)(offX + p2.first * scale), (int)(offY + p2.second * scale));
                } else if (geo.type == GeoEntity::CIRCLE) {
                    auto center = comp.localToGlobal(geo.x1, geo.y1);
                    double r = geo.radius;

                    int cx = (int)(offX + center.first * scale);
                    int cy = (int)(offY + center.second * scale);
                    int cr = (int)(r * scale);

                    HGDIOBJ oldBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    Ellipse(memDC, cx - cr, cy - cr, cx + cr, cy + cr);
                    SelectObject(memDC, oldBrush);
                } else if (geo.type == GeoEntity::ARC) {
                    auto center = comp.localToGlobal(geo.x1, geo.y1);
                    double r = geo.radius;

                    int cx = (int)(offX + center.first * scale);
                    int cy = (int)(offY + center.second * scale);
                    int cr = (int)(r * scale);

                    double sa_rad = geo.start_angle * M_PI / 180.0;
                    double ea_rad = geo.end_angle * M_PI / 180.0;

                    double lx_start = geo.x1 + r * std::cos(sa_rad);
                    double ly_start = geo.y1 + r * std::sin(sa_rad);
                    double lx_end   = geo.x1 + r * std::cos(ea_rad);
                    double ly_end   = geo.y1 + r * std::sin(ea_rad);

                    auto g_start = comp.localToGlobal(lx_start, ly_start);
                    auto g_end   = comp.localToGlobal(lx_end, ly_end);

                    Arc(memDC, cx - cr, cy - cr, cx + cr, cy + cr,
                        (int)(offX + g_start.first * scale), (int)(offY + g_start.second * scale),
                        (int)(offX + g_end.first * scale), (int)(offY + g_end.second * scale));
                }
            }

            DeleteObject(pathPen);
            count++;
        }
    } else {
        // Technical logo splash
        HFONT logoFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Courier New");
        SelectObject(memDC, logoFont);
        SetTextColor(memDC, RGB(200, 200, 200));
        SetBkMode(memDC, TRANSPARENT);

        std::string title = "[ NESTINGATOR3000 ENGINE ACTIVE ]";
        TextOut(memDC, 30, 30, title.c_str(), static_cast<int>(title.size()));

        std::string subtitle = "Zaimportuj pliki DXF lub dodaj detale ręcznie.";
        TextOut(memDC, 30, 65, subtitle.c_str(), static_cast<int>(subtitle.size()));

        DeleteObject(logoFont);
    }

    // Blit background buffer into actual control HDC
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

    // RestoreDC completely cleans up all selected pens, brushes, fonts, and saves GDI handles perfectly
    RestoreDC(memDC, saveId);

    // Clean up DC bitmap selection & resources
    SelectObject(memDC, oldBM);
    DeleteObject(memBM);
    DeleteDC(memDC);
}
