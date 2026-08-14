#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h> // Explicitly include for OPENFILENAME, GetOpenFileName and GetSaveFileName
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include "ComponentManager.h"
#include "NestingEngine.h"
#include "NCGenerator.h"

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    // Create and initialize the window and child controls
    bool Create(HINSTANCE hInstance, int nCmdShow);

    // Run the Win32 message loop
    int Run();

private:
    // Window Procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Handle messages
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Initialize child controls (Panels, ListView, Labels, Edit boxes, Buttons)
    void InitControls(HWND hwnd);

    // Event handlers
    void OnAddComponent();
    void OnRemoveComponent();
    void OnImportDXF();
    void OnRunNesting();
    void OnExportGCode();
    void OnPaintCanvas(HWND hwnd, HDC hdc);

    // Redraw and recalculate UI layout size/positions
    void ResizeControls(int width, int height);

    // Refresh ListView with the current components
    void RefreshListView();

    // Member Variables
    HWND m_hwnd;
    HINSTANCE m_hInstance;

    // Menu bar
    HMENU m_hMenu;

    // Top Toolbar
    HWND m_hToolbar;

    // LEFT PANEL - BAZA DETALI
    HWND m_hGrpBazaDetali;
    HWND m_hTreeViewDB;
    HWND m_hEditSearch;
    HWND m_hBtnSearchPencil;
    HWND m_hBtnSearchPlay;
    HWND m_hBtnReset;
    HWND m_hBtnPlusMinus;
    HWND m_hBtnScale;

    // LEFT PANEL - KOMPONENTY
    HWND m_hGrpKomponenty;
    HWND m_hListViewComp;
    HWND m_hBtnAddComp;
    HWND m_hBtnRemoveComp;
    HWND m_hBtnMoveUp;
    HWND m_hBtnMoveDown;

    // RIGHT PANEL - PODGLAD PLYTY
    HWND m_hGrpPodgladPlyty;
    HWND m_hCanvas;

    // Path simulation slider
    HWND m_hSlider;
    int m_simPercent;

    // RIGHT PANEL - PARAMETRY
    HWND m_hGrpParametry;
    HWND m_hEditPlateW;
    HWND m_hEditMargin;
    HWND m_hEditSpacing;

    // Custom Checkboxes for Rotations (0, 90, 180, 270)
    HWND m_hChkRot0;
    HWND m_hChkRot90;
    HWND m_hChkRot180;
    HWND m_hChkRot270;

    // Custom Angle Step edit box
    HWND m_hEditAngleStep;

    // NC parameters
    HWND m_hComboTool;
    HWND m_hEditFeed;

    // Action button
    HWND m_hBtnGenerateNesting;

    // Status bar
    HWND m_hStatusBar;

    // Business Logic Instances
    ComponentManager m_compManager;
    std::vector<SheetLayout> m_sheets;
    NestingParams m_nestingParams;
    NCParams m_ncParams;
    int m_activeSheetIndex;

    // Thread safety flag for active calculation
    bool m_isNestingRunning;
    std::thread m_bgThread;

    // Performance measurement
    double m_lastNestingTime;
};

#endif // MAINWINDOW_H
