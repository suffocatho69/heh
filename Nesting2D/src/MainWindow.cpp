#include "MainWindow.h"
#include "resource.h"
#include "DXFReader.h"
#include <sstream>
#include <iomanip>
#include <cmath>

// Win32 Controls Identifiers
#define IDC_LIST_COMPONENTS   1001
#define IDC_BTN_ADD           1002
#define IDC_BTN_REMOVE        1003
#define IDC_BTN_IMPORT        1004

#define IDC_EDIT_PLATEW       1005
#define IDC_EDIT_PLATEH       1006
#define IDC_EDIT_MARGIN       1007
#define IDC_EDIT_SPACING      1008
#define IDC_CHK_ROTATION      1009

#define IDC_EDIT_SPINDLE      1010
#define IDC_EDIT_FEEDCUT      1011
#define IDC_EDIT_FEEDPLUNGE   1012
#define IDC_EDIT_SAFEZ        1013
#define IDC_EDIT_CUTDEPTH     1014

#define IDC_BTN_NESTING       1015
#define IDC_BTN_EXPORTG       1016

#define IDC_COMBO_LAYOUTS     1017
#define IDC_CANVAS            1018
#define IDC_STATIC_STATS      1019

// Global class name
const char g_szClassName[] = "Nesting2DMainWindowClass";

MainWindow::MainWindow()
    : m_hwnd(NULL), m_hInstance(NULL), m_hListView(NULL), m_hBtnAdd(NULL), m_hBtnRemove(NULL),
      m_hBtnImport(NULL), m_hEditPlateW(NULL), m_hEditPlateH(NULL), m_hEditMargin(NULL),
      m_hEditSpacing(NULL), m_hChkRotation(NULL), m_hEditSpindle(NULL), m_hEditFeedCut(NULL),
      m_hEditFeedPlunge(NULL), m_hEditSafeZ(NULL), m_hEditCutDepth(NULL), m_hBtnNesting(NULL),
      m_hBtnExportG(NULL), m_hComboLayouts(NULL), m_hCanvas(NULL), m_hStaticStats(NULL),
      m_activeSheetIndex(-1) {

    // Seed default components for retro feel
    m_compManager.addComponent(Component("Detala_A", 120.0, 80.0, 6));
    m_compManager.addComponent(Component("Detala_B", 250.0, 100.0, 4));
    m_compManager.addComponent(Component("Detala_C", 60.0, 60.0, 12));
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
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); // Win98 look
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Classic Retro main frame dimensions
    m_hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "RETRO 2D NESTING SYSTEM - Rozkroj Detali z DXF na Plytach",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 700,
        NULL, NULL, m_hInstance, this
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
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            switch (wmId) {
                case IDC_BTN_ADD:
                    OnAddComponent();
                    break;
                case IDC_BTN_REMOVE:
                    OnRemoveComponent();
                    break;
                case IDC_BTN_IMPORT:
                    OnImportDXF();
                    break;
                case IDC_BTN_NESTING:
                    OnRunNesting();
                    break;
                case IDC_BTN_EXPORTG:
                    OnExportGCode();
                    break;
                case IDC_COMBO_LAYOUTS: {
                    if (wmEvent == CBN_SELCHANGE) {
                        int index = SendMessage(m_hComboLayouts, CB_GETCURSEL, 0, 0);
                        if (index != CB_ERR) {
                            m_activeSheetIndex = index;
                            InvalidateRect(m_hCanvas, NULL, TRUE);
                        }
                    }
                    break;
                }
            }
            break;
        }
        case WM_DRAWITEM: {
            // Handle painting of SS_OWNERDRAW controls (the Canvas)
            DRAWITEMSTRUCT* pDIS = (DRAWITEMSTRUCT*)lParam;
            if (pDIS->CtlID == IDC_CANVAS) {
                OnPaintCanvas(pDIS->hwndItem, pDIS->hDC);
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
    // Fonts for Retro Aesthetic
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

    // LEFT PANEL: Component List Group Box
    HWND hGrpComponents = CreateWindowEx(0, "BUTTON", "Lista Komponentów (X / Y / Ilość)",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        10, 10, 280, 640, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hGrpComponents, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Listview for Components
    m_hListView = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        20, 35, 260, 480, hwnd, (HMENU)IDC_LIST_COMPONENTS, m_hInstance, NULL);
    SendMessage(m_hListView, WM_SETFONT, (WPARAM)hFont, TRUE);
    ListView_SetExtendedListViewStyle(m_hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // Add list columns
    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    lvc.cx = 90;
    lvc.pszText = (LPSTR)"Nazwa";
    ListView_InsertColumn(m_hListView, 0, &lvc);

    lvc.cx = 50;
    lvc.pszText = (LPSTR)"Szer (X)";
    ListView_InsertColumn(m_hListView, 1, &lvc);

    lvc.cx = 50;
    lvc.pszText = (LPSTR)"Wys (Y)";
    ListView_InsertColumn(m_hListView, 2, &lvc);

    lvc.cx = 40;
    lvc.pszText = (LPSTR)"Ilość";
    ListView_InsertColumn(m_hListView, 3, &lvc);

    // Buttons under ListView
    m_hBtnAdd = CreateWindowEx(0, "BUTTON", "Dodaj Ręcznie",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 525, 125, 30, hwnd, (HMENU)IDC_BTN_ADD, m_hInstance, NULL);
    SendMessage(m_hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hBtnRemove = CreateWindowEx(0, "BUTTON", "Usuń Zaznaczony",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        155, 525, 125, 30, hwnd, (HMENU)IDC_BTN_REMOVE, m_hInstance, NULL);
    SendMessage(m_hBtnRemove, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hBtnImport = CreateWindowEx(0, "BUTTON", "Importuj plik DXF...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 565, 260, 35, hwnd, (HMENU)IDC_BTN_IMPORT, m_hInstance, NULL);
    SendMessage(m_hBtnImport, WM_SETFONT, (WPARAM)hFont, TRUE);


    // MIDDLE PANEL: Settings Group Box (Parametry płyty i CNC)
    HWND hGrpSettings = CreateWindowEx(0, "BUTTON", "Parametry Płyty i Generatora NC",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        300, 10, 300, 640, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hGrpSettings, WM_SETFONT, (WPARAM)hFont, TRUE);

    int startY = 35;
    int gapY = 25;

    // Helper lambda for creating labels and edit controls
    auto AddSettingField = [&](const std::string& labelText, const std::string& defaultVal, int editId, HWND& outEdit) {
        HWND hLabel = CreateWindowEx(0, "STATIC", labelText.c_str(),
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            310, startY + 3, 150, 20, hwnd, NULL, m_hInstance, NULL);
        SendMessage(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

        outEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", defaultVal.c_str(),
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER,
            470, startY, 110, 20, hwnd, (HMENU)(INT_PTR)editId, m_hInstance, NULL);
        SendMessage(outEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        startY += gapY;
    };

    // Plate Settings
    AddSettingField("Szerokość płyty [mm]:", "2000", IDC_EDIT_PLATEW, m_hEditPlateW);
    AddSettingField("Wysokość płyty [mm]:", "1000", IDC_EDIT_PLATEH, m_hEditPlateH);
    AddSettingField("Margines krawędzi [mm]:", "10", IDC_EDIT_MARGIN, m_hEditMargin);
    AddSettingField("Odstęp detali [mm]:", "5", IDC_EDIT_SPACING, m_hEditSpacing);

    // Rotation selection checkbox
    m_hChkRotation = CreateWindowEx(0, "BUTTON", "Zezwalaj na obrót o 90°",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        470, startY, 200, 20, hwnd, (HMENU)IDC_CHK_ROTATION, m_hInstance, NULL);
    SendMessage(m_hChkRotation, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hChkRotation, BM_SETCHECK, BST_CHECKED, 0);

    startY += gapY + 15;

    // CNC parameters
    AddSettingField("Obroty wrzeciona [RPM]:", "12000", IDC_EDIT_SPINDLE, m_hEditSpindle);
    AddSettingField("Posuw roboczy [mm/min]:", "1500", IDC_EDIT_FEEDCUT, m_hEditFeedCut);
    AddSettingField("Posuw zagłębiania [mm/min]:", "500", IDC_EDIT_FEEDPLUNGE, m_hEditFeedPlunge);
    AddSettingField("Wysokość bezpieczna Z [mm]:", "10", IDC_EDIT_SAFEZ, m_hEditSafeZ);
    AddSettingField("Głębokość cięcia Z [mm]:", "-2", IDC_EDIT_CUTDEPTH, m_hEditCutDepth);

    // Large Action Buttons
    m_hBtnNesting = CreateWindowEx(0, "BUTTON", "URUCHOM NESTING (G-code)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        310, startY + 10, 270, 45, hwnd, (HMENU)IDC_BTN_NESTING, m_hInstance, NULL);
    SendMessage(m_hBtnNesting, WM_SETFONT, (WPARAM)CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif"), TRUE);

    m_hBtnExportG = CreateWindowEx(0, "BUTTON", "Eksportuj plik .NC (G-kod)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        310, startY + 65, 270, 40, hwnd, (HMENU)IDC_BTN_EXPORTG, m_hInstance, NULL);
    SendMessage(m_hBtnExportG, WM_SETFONT, (WPARAM)hFont, TRUE);
    EnableWindow(m_hBtnExportG, FALSE); // Disabled until Nesting is processed


    // RIGHT PANEL: Visualization & Statistics
    HWND hGrpRight = CreateWindowEx(0, "BUTTON", "Podgląd i Statystyki Rozkroju",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        610, 10, 390, 640, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hGrpRight, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Dropdown for Active plate
    HWND hLabelCombo = CreateWindowEx(0, "STATIC", "Wybierz układ płyty:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        625, 38, 120, 20, hwnd, NULL, m_hInstance, NULL);
    SendMessage(hLabelCombo, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hComboLayouts = CreateWindowEx(0, "COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        750, 35, 230, 150, hwnd, (HMENU)IDC_COMBO_LAYOUTS, m_hInstance, NULL);
    SendMessage(m_hComboLayouts, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Static label for detailed statistics
    m_hStaticStats = CreateWindowEx(0, "STATIC", "Brak wygenerowanego rozkroju. Uruchom algorytm nesting.",
        WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS | SS_SUNKEN,
        625, 65, 355, 80, hwnd, (HMENU)IDC_STATIC_STATS, m_hInstance, NULL);
    SendMessage(m_hStaticStats, WM_SETFONT, (WPARAM)hFont, TRUE);

    // GDI Drawing canvas (with SS_OWNERDRAW)
    m_hCanvas = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "",
        WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        625, 155, 355, 480, hwnd, (HMENU)IDC_CANVAS, m_hInstance, NULL);
}

void MainWindow::ResizeControls(int width, int height) {
    if (m_hCanvas) {
        int rightWidth = width - 620;
        int bottomHeight = height - 170;
        if (rightWidth < 200) rightWidth = 200;
        if (bottomHeight < 150) bottomHeight = 150;

        MoveWindow(m_hCanvas, 620, 155, rightWidth - 15, bottomHeight, TRUE);
    }
}

void MainWindow::RefreshListView() {
    ListView_DeleteAllItems(m_hListView);

    const auto& comps = m_compManager.getComponents();
    for (size_t i = 0; i < comps.size(); ++i) {
        const auto& c = comps[i];

        LVITEM lvi;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = static_cast<int>(i);
        lvi.iSubItem = 0;
        lvi.pszText = (LPSTR)c.name.c_str();
        lvi.lParam = (LPARAM)i;
        ListView_InsertItem(m_hListView, &lvi);

        std::string wStr = std::to_string((int)c.width);
        ListView_SetItemText(m_hListView, static_cast<int>(i), 1, (LPSTR)wStr.c_str());

        std::string hStr = std::to_string((int)c.height);
        ListView_SetItemText(m_hListView, static_cast<int>(i), 2, (LPSTR)hStr.c_str());

        std::string qStr = std::to_string(c.quantity);
        ListView_SetItemText(m_hListView, static_cast<int>(i), 3, (LPSTR)qStr.c_str());
    }
}

void MainWindow::OnAddComponent() {
    Component manual("Nowy_Detal", 150.0, 150.0, 2);
    m_compManager.addComponent(manual);
    RefreshListView();
}

void MainWindow::OnRemoveComponent() {
    int idx = ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
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
            dxfComp.quantity = 5; // Default quantity for nested layout
            m_compManager.addComponent(dxfComp);
            RefreshListView();
            MessageBox(m_hwnd, "Plik DXF zaimportowany pomyślnie!", "Sukces", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(m_hwnd, "Nie udało się zaimportować pliku DXF. Plik może być uszkodzony lub pusty.", "Błąd", MB_OK | MB_ICONERROR);
        }
    }
}

void MainWindow::OnRunNesting() {
    char buf[64];

    GetWindowText(m_hEditPlateW, buf, sizeof(buf));
    m_nestingParams.sheetWidth = atof(buf);
    if (m_nestingParams.sheetWidth <= 0) m_nestingParams.sheetWidth = 2000;

    GetWindowText(m_hEditPlateH, buf, sizeof(buf));
    m_nestingParams.sheetHeight = atof(buf);
    if (m_nestingParams.sheetHeight <= 0) m_nestingParams.sheetHeight = 1000;

    GetWindowText(m_hEditMargin, buf, sizeof(buf));
    m_nestingParams.margin = atof(buf);

    GetWindowText(m_hEditSpacing, buf, sizeof(buf));
    m_nestingParams.spacing = atof(buf);

    m_nestingParams.allowRotation = (SendMessage(m_hChkRotation, BM_GETCHECK, 0, 0) == BST_CHECKED);

    // Read NC parameters
    GetWindowText(m_hEditSpindle, buf, sizeof(buf));
    m_ncParams.spindleSpeed = atof(buf);

    GetWindowText(m_hEditFeedCut, buf, sizeof(buf));
    m_ncParams.cuttingFeed = atof(buf);

    GetWindowText(m_hEditFeedPlunge, buf, sizeof(buf));
    m_ncParams.plungeFeed = atof(buf);

    GetWindowText(m_hEditSafeZ, buf, sizeof(buf));
    m_ncParams.safeZ = atof(buf);

    GetWindowText(m_hEditCutDepth, buf, sizeof(buf));
    m_ncParams.cutDepth = atof(buf);

    // Run nesting logic
    m_sheets = NestingEngine::performNesting(m_compManager.getComponents(), m_nestingParams);

    if (m_sheets.empty()) {
        MessageBox(m_hwnd, "Brak elementów do ułożenia lub elementy za duże!", "Nesting Błąd", MB_OK | MB_ICONWARNING);
        EnableWindow(m_hBtnExportG, FALSE);
        return;
    }

    // Success! Update active layout selection
    m_activeSheetIndex = 0;
    RefreshLayoutSelector();
    EnableWindow(m_hBtnExportG, TRUE);

    // Calculate statistics
    int totalOriginal = 0;
    for (const auto& c : m_compManager.getComponents()) {
        totalOriginal += c.quantity;
    }
    NestingStats stats = ComponentManager::calculateStats(m_sheets, totalOriginal);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "STATYSTYKI NESTINGU:\n";
    ss << "• Średnie zużycie materiału: " << stats.materialUtilization << "%\n";
    ss << "• Liczba płyt ogółem: " << stats.totalSheets << " szt.\n";
    ss << "• Liczba unikalnych układów: " << stats.uniqueLayouts << "\n";
    ss << "• Detale ułożone: " << stats.totalPlacedCount << " / " << totalOriginal;

    if (stats.totalUnplacedCount > 0) {
        ss << " (" << stats.totalUnplacedCount << " nie zmieściło się)";
    }

    SetWindowText(m_hStaticStats, ss.str().c_str());

    // Force canvas paint
    InvalidateRect(m_hCanvas, NULL, TRUE);
}

void MainWindow::RefreshLayoutSelector() {
    SendMessage(m_hComboLayouts, CB_RESETCONTENT, 0, 0);

    for (size_t i = 0; i < m_sheets.size(); ++i) {
        std::string name = "Płyta #" + std::to_string(i + 1) + " ("
            + std::to_string((int)m_sheets[i].placedComponents.size()) + " szt. | "
            + std::to_string((int)std::round(m_sheets[i].materialUtilization)) + "%)";
        SendMessage(m_hComboLayouts, CB_ADDSTRING, 0, (LPARAM)name.c_str());
    }
    SendMessage(m_hComboLayouts, CB_SETCURSEL, 0, 0);
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

    // Double buffering using MemDC to prevent retro flicker
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBM = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
    SelectObject(memDC, memBM);

    // 1. Draw Canvas background (Dark dark gray - retro aesthetic)
    HBRUSH bgBrush = CreateSolidBrush(RGB(30, 30, 30));
    FillRect(memDC, &rect, bgBrush);
    DeleteObject(bgBrush);

    // If sheets exist, draw the selected layout
    if (m_activeSheetIndex >= 0 && m_activeSheetIndex < static_cast<int>(m_sheets.size())) {
        const auto& sheet = m_sheets[m_activeSheetIndex];

        // 2. Compute scale factor to fit sheet to canvas rect
        double pad = 20.0;
        double canvasW = rect.right - 2 * pad;
        double canvasH = rect.bottom - 2 * pad;

        double scaleX = canvasW / sheet.width;
        double scaleY = canvasH / sheet.height;
        double scale = (scaleX < scaleY) ? scaleX : scaleY; // Keep aspect ratio

        // Margins to center sheet on canvas
        double offX = pad + (canvasW - sheet.width * scale) / 2.0;
        double offY = pad + (canvasH - sheet.height * scale) / 2.0;

        // Draw Sheet boundaries (Bright Retro green frame)
        HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
        SelectObject(memDC, borderPen);

        // Draw sheet white semi-transparent/gray fill
        HBRUSH sheetBrush = CreateSolidBrush(RGB(50, 50, 50));
        SelectObject(memDC, sheetBrush);

        Rectangle(memDC,
                  (int)offX,
                  (int)offY,
                  (int)(offX + sheet.width * scale),
                  (int)(offY + sheet.height * scale));

        DeleteObject(sheetBrush);
        DeleteObject(borderPen);

        // Draw Sheet grid coordinate lines (thin darker green)
        HPEN gridPen = CreatePen(PS_DOT, 1, RGB(0, 100, 0));
        SelectObject(memDC, gridPen);
        for (double gx = 200.0; gx < sheet.width; gx += 200.0) {
            MoveToEx(memDC, (int)(offX + gx * scale), (int)offY, NULL);
            LineTo(memDC, (int)(offX + gx * scale), (int)(offY + sheet.height * scale));
        }
        for (double gy = 200.0; gy < sheet.height; gy += 200.0) {
            MoveToEx(memDC, (int)offX, (int)(offY + gy * scale), NULL);
            LineTo(memDC, (int)(offX + sheet.width * scale), (int)(offY + gy * scale));
        }
        DeleteObject(gridPen);

        // 3. Draw nested components
        HPEN compPen = CreatePen(PS_SOLID, 2, RGB(255, 200, 0)); // Retro Gold border
        HBRUSH compBrush = CreateSolidBrush(RGB(80, 80, 120));   // Muted cyan/blue-gray fill

        SelectObject(memDC, compPen);
        SelectObject(memDC, compBrush);

        for (const auto& comp : sheet.placedComponents) {
            double ew = comp.getEffectiveWidth();
            double eh = comp.getEffectiveHeight();

            // Screen coords for component bounding box
            int sx = (int)(offX + comp.posX * scale);
            int sy = (int)(offY + comp.posY * scale);
            int sw = (int)(ew * scale);
            int sh = (int)(eh * scale);

            // Draw bounding rect
            Rectangle(memDC, sx, sy, sx + sw, sy + sh);

            // Draw internal geometry pathways if present
            HPEN pathPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); // White cutlines
            HGDIOBJ oldPen = SelectObject(memDC, pathPen);

            for (const auto& geo : comp.geometry) {
                if (geo.type == GeoEntity::LINE) {
                    auto p1 = comp.localToGlobal(geo.x1, geo.y1);
                    auto p2 = comp.localToGlobal(geo.x2, geo.y2);

                    int g1x = (int)(offX + p1.first * scale);
                    int g1y = (int)(offY + p1.second * scale);
                    int g2x = (int)(offX + p2.first * scale);
                    int g2y = (int)(offY + p2.second * scale);

                    MoveToEx(memDC, g1x, g1y, NULL);
                    LineTo(memDC, g2x, g2y);
                } else if (geo.type == GeoEntity::CIRCLE) {
                    auto center = comp.localToGlobal(geo.x1, geo.y1);
                    double r = geo.radius;

                    int cx = (int)(offX + center.first * scale);
                    int cy = (int)(offY + center.second * scale);
                    int cr = (int)(r * scale);

                    // Create brush hollow for internal circles
                    HGDIOBJ oldBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    Ellipse(memDC, cx - cr, cy - cr, cx + cr, cy + cr);
                    SelectObject(memDC, oldBrush);
                } else if (geo.type == GeoEntity::ARC) {
                    auto center = comp.localToGlobal(geo.x1, geo.y1);
                    double r = geo.radius;

                    int cx = (int)(offX + center.first * scale);
                    int cy = (int)(offY + center.second * scale);
                    int cr = (int)(r * scale);

                    int left   = cx - cr;
                    int top    = cy - cr;
                    int right  = cx + cr;
                    int bottom = cy + cr;

                    double sa_rad = geo.start_angle * M_PI / 180.0;
                    double ea_rad = geo.end_angle * M_PI / 180.0;

                    double lx_start = geo.x1 + r * std::cos(sa_rad);
                    double ly_start = geo.y1 + r * std::sin(sa_rad);
                    double lx_end   = geo.x1 + r * std::cos(ea_rad);
                    double ly_end   = geo.y1 + r * std::sin(ea_rad);

                    auto g_start = comp.localToGlobal(lx_start, ly_start);
                    auto g_end   = comp.localToGlobal(lx_end, ly_end);

                    int g_sx = (int)(offX + g_start.first * scale);
                    int g_sy = (int)(offY + g_start.second * scale);
                    int g_ex = (int)(offX + g_end.first * scale);
                    int g_ey = (int)(offY + g_end.second * scale);

                    Arc(memDC, left, top, right, bottom, g_sx, g_sy, g_ex, g_ey);
                }
            }

            SelectObject(memDC, oldPen);
            DeleteObject(pathPen);

            // Draw small name label in the center
            HFONT textFont = CreateFont(11, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Courier New");
            HGDIOBJ oldFont = SelectObject(memDC, textFont);
            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);

            std::string labelText = comp.name;
            if (comp.rotated) labelText += " [90]";

            TextOut(memDC, sx + 5, sy + 5, labelText.c_str(), static_cast<int>(labelText.size()));
            SelectObject(memDC, oldFont);
            DeleteObject(textFont);
        }

        DeleteObject(compPen);
        DeleteObject(compBrush);

    } else {
        // Draw Retro welcome text on canvas
        HFONT textFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Courier New");
        HGDIOBJ oldFont = SelectObject(memDC, textFont);
        SetTextColor(memDC, RGB(0, 255, 0));
        SetBkMode(memDC, TRANSPARENT);

        std::string welcome = "SYSTEM RETRO 2D NESTING READY";
        TextOut(memDC, 30, 30, welcome.c_str(), static_cast<int>(welcome.size()));

        std::string prompt = "Wczytaj komponenty i uruchom nesting...";
        TextOut(memDC, 30, 60, prompt.c_str(), static_cast<int>(prompt.size()));

        SelectObject(memDC, oldFont);
        DeleteObject(textFont);
    }

    // Blit background buffer into actual control HDC
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

    // Cleanup resources
    DeleteObject(memBM);
    DeleteDC(memDC);
}
