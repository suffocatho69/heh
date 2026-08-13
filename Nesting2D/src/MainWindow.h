#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h> // Explicitly include for OPENFILENAME, GetOpenFileName and GetSaveFileName
#include <string>
#include <vector>
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

    // Refresh layout listbox/combobox based on sheets
    void RefreshLayoutSelector();

    // Member Variables
    HWND m_hwnd;
    HINSTANCE m_hInstance;

    // Controls
    HWND m_hListView;          // Component List
    HWND m_hBtnAdd;            // Add manually Button
    HWND m_hBtnRemove;         // Remove Button
    HWND m_hBtnImport;         // Import DXF Button

    // Base plate parameters
    HWND m_hEditPlateW;        // Sheet width
    HWND m_hEditPlateH;        // Sheet height
    HWND m_hEditMargin;        // Margins
    HWND m_hEditSpacing;       // Minimum spacing
    HWND m_hChkRotation;       // Allow Rotation checkbox

    // CNC Machine Parameters
    HWND m_hEditSpindle;       // Spindle Speed
    HWND m_hEditFeedCut;       // Cutting Feed
    HWND m_hEditFeedPlunge;    // Plunge Feed
    HWND m_hEditSafeZ;         // Safe Z Height
    HWND m_hEditCutDepth;      // Cutting Depth

    // Action buttons
    HWND m_hBtnNesting;        // Perform Nesting Button
    HWND m_hBtnExportG;        // Export G-code Button

    // Visualization and Layout stats
    HWND m_hComboLayouts;      // ComboBox selector for active Sheet index
    HWND m_hCanvas;            // Static custom control for drawing GDI shapes
    HWND m_hStaticStats;       // Label displaying material utilization and sheet stats

    // Business Logic Instances
    ComponentManager m_compManager;
    std::vector<SheetLayout> m_sheets;
    NestingParams m_nestingParams;
    NCParams m_ncParams;
    int m_activeSheetIndex;
};

#endif // MAINWINDOW_H
