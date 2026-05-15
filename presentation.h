#pragma once

#include "logic.h"
#include "data.h"
#include "raylib.h"

#include <string>
#include <vector>

 // ─────────────────────────────────────────────
 //  Window / Layout constants
 // ─────────────────────────────────────────────

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 800;
const int SIDEBAR_WIDTH = 220;
const int TOPBAR_HEIGHT = 60;
const int CARD_PADDING = 16;
const int ROW_HEIGHT = 44;
const int TARGET_FPS = 60;

// ─────────────────────────────────────────────
//  Color palette  (Shkolo-inspired, modern)
// ─────────────────────────────────────────────

// Main brand colors
extern Color COL_BG;           // Page background
extern Color COL_SURFACE;      // Card / panel background
extern Color COL_SURFACE2;     // Secondary surface (sidebar, header)
extern Color COL_BORDER;       // Subtle border
extern Color COL_PRIMARY;      // Brand blue
extern Color COL_PRIMARY_DARK; // Darker blue for hover
extern Color COL_ACCENT;       // Accent teal/green
extern Color COL_TEXT;         // Primary text
extern Color COL_TEXT_MUTED;   // Secondary / muted text
extern Color COL_TEXT_LIGHT;   // Text on dark backgrounds

// Grade colors (index matches logic_gradeColorIndex)
extern Color COL_GRADE[5];
// 0=red, 1=orange, 2=yellow, 3=light-green, 4=green

// ─────────────────────────────────────────────
//  Application state
// ─────────────────────────────────────────────

// Which screen / view is currently active
enum class Screen
{
    LOGIN = -1,   // Login screen (shown before everything else)
    DASHBOARD = 0,
    STUDENTS = 1,
    GRADES = 2,
    SEARCH = 3,
    SORT_DEMO = 4,
    STATS = 5,
    ABOUT = 6
};

// Modal dialog types
enum class Modal
{
    NONE = 0,
    ADD_STUDENT = 1,
    EDIT_STUDENT = 2,
    DELETE_CONFIRM = 3,
    ADD_GRADE = 4,
    EDIT_GRADE = 5,
    STUDENT_DETAIL = 6,
    NOTIFICATION = 7
};

// Input buffer sizes
const int INPUT_BUF = 128;

/*
 * AppState – centralised mutable state passed to every render function.
 * The presentation layer owns this; logic and data layers do not touch it.
 */
struct AppState
{
    // ── Core data ─────────────────────────────────────────────
    DataStore store;

    // ── Authentication ────────────────────────────────────────
    UserStore        userStore;
    const User* loggedInUser = nullptr;
    char             loginUserBuf[INPUT_BUF] = {};
    char             loginPassBuf[INPUT_BUF] = {};
    std::string      loginError = "";
    bool             loginShowPass = false;
    float            loginShakeTimer = 0.0f;

    // ── Navigation ────────────────────────────────────────────
    Screen    currentScreen = Screen::LOGIN;
    Modal     activeModal = Modal::NONE;

    // ── Student list state ────────────────────────────────────
    std::vector<Student*> displayedStudents;
    int     selectedStudentId = -1;
    int     tableScrollY = 0;
    int     tableHoverRow = -1;

    SortField sortField = SortField::BY_LAST_NAME;
    SortOrder sortOrder = SortOrder::ASCENDING;

    // ── Search state ──────────────────────────────────────────
    char    searchBuf[INPUT_BUF] = {};
    bool    searchDirty = false;
    std::vector<Student*> searchResults;
    int     lastBinaryIdx = -1;
    bool    searchUseBinary = false;

    // ── Grade entry state ─────────────────────────────────────
    int         gradeStudentId = -1;
    Subject     gradeSubject = Subject::MATHEMATICS;
    char        gradeValueBuf[16] = {};
    char        gradeNoteBuf[INPUT_BUF] = {};
    std::string gradeError = "";

    // ── Add/Edit student form ─────────────────────────────────
    char    formFirst[INPUT_BUF] = {};
    char    formLast[INPUT_BUF] = {};
    char    formEgn[INPUT_BUF] = {};
    int     formGender = 0;
    int     formYearGroup = 2;
    char    formClass[INPUT_BUF] = {};
    int     editStudentId = -1;
    std::string formError = "";

    // ── Delete confirm ────────────────────────────────────────
    int     deleteTargetId = -1;

    // ── Sort demo ─────────────────────────────────────────────
    SortField demoSortField = SortField::BY_LAST_NAME;
    SortOrder demoSortOrder = SortOrder::ASCENDING;
    bool      demoUseBubble = true;
    int       demoSwapCount = 0;
    std::vector<Student*> demoList;

    // ── Statistics ────────────────────────────────────────────
    std::string statsClassFilter = "";
    GroupStats  currentStats;
    bool        statsNeedsRefresh = true;

    // ── Notification ──────────────────────────────────────────
    std::string notifMessage = "";
    Color       notifColor = { 46, 204, 113, 255 };
    float       notifTimer = 0.0f;

    // ── Active text-box focus tracking ───────────────────────
    int focusedInput = 0;

    // ── Filter ────────────────────────────────────────────────
    char filterClass[INPUT_BUF] = {};
};

// ─────────────────────────────────────────────
//  Function Prototypes – Lifecycle
// ─────────────────────────────────────────────

// Initialise the application state with defaults and seed data
void pres_init(AppState& app);

// Main game loop tick: process input + render everything
void pres_update(AppState& app);

// Clean up resources on shutdown
void pres_shutdown(AppState& app);

// ─────────────────────────────────────────────
//  Function Prototypes – Screens
// ─────────────────────────────────────────────

void pres_drawTopBar(AppState& app);
void pres_drawSidebar(AppState& app);
void pres_drawLoginScreen(AppState& app);
void pres_drawDashboard(AppState& app);
void pres_drawStudentsScreen(AppState& app);
void pres_drawGradesScreen(AppState& app);
void pres_drawSearchScreen(AppState& app);
void pres_drawSortDemoScreen(AppState& app);
void pres_drawStatsScreen(AppState& app);
void pres_drawAboutScreen(AppState& app);

// ─────────────────────────────────────────────
//  Function Prototypes – Modals
// ─────────────────────────────────────────────

void pres_drawModalAddStudent(AppState& app);
void pres_drawModalEditStudent(AppState& app);
void pres_drawModalDeleteConfirm(AppState& app);
void pres_drawModalAddGrade(AppState& app);
void pres_drawModalStudentDetail(AppState& app);
void pres_drawModalNotification(AppState& app);

// ─────────────────────────────────────────────
//  Function Prototypes – Reusable UI Primitives
// ─────────────────────────────────────────────

// Draw a filled rounded rectangle
void pres_drawCard(Rectangle rect, Color bg, Color border, float radius);

// Draw a centred label inside a rectangle
void pres_drawLabel(Rectangle rect, const std::string& text,
    Color color, int fontSize, bool centred = false);

// Draw a primary button; returns true if clicked
bool pres_button(Rectangle rect, const std::string& label,
    Color bg, Color textColor, int fontSize = 16);

// Draw a secondary/ghost button
bool pres_buttonGhost(Rectangle rect, const std::string& label, int fontSize = 16);

// Draw a single-line text input box; returns true when Enter is pressed
bool pres_textInput(Rectangle rect, char* buf, int bufSize,
    const std::string& placeholder, int& focusId,
    int thisId, int fontSize = 15);

// Draw a grade badge (coloured pill with the grade value or label)
void pres_gradeBadge(float x, float y, double grade, int fontSize = 14);

// Draw a progress bar (value in [0,1])
void pres_progressBar(Rectangle rect, float value, Color fill, Color bg);

// Draw a tooltip-style floating label near the mouse
void pres_tooltip(const std::string& text);

// Show a brief notification (auto-dismisses after ~2 s)
void pres_notify(AppState& app, const std::string& message,
    Color color = { 46, 204, 113, 255 });

// Draw the stat card (icon + big number + label)
void pres_statCard(Rectangle rect, const std::string& valueStr,
    const std::string& label, Color accent);

// Draw the grade colour legend
void pres_drawGradeLegend(float x, float y);

// Helper: draw a horizontal separator line
void pres_separator(float x, float y, float width, Color color);

// Helper: draw a section header inside a panel
void pres_sectionHeader(float x, float y, float width,
    const std::string& title, Color color);

// Draw a drop-down style selector (cycles through options on click)
// Returns true if the value changed
bool pres_cycleSelector(Rectangle rect, const std::string& label,
    const std::vector<std::string>& options,
    int& selectedIndex);

// ─────────────────────────────────────────────
//  Utility / helpers used by presentation layer
// ─────────────────────────────────────────────

// Format a double to 2 decimal places as std::string
std::string pres_fmt2(double value);

// Format a grade with its Bulgarian label: "5.50 – Отличен"
std::string pres_fmtGrade(double average);

// Convert a raylib Color to a slightly lighter shade
Color pres_lighter(Color c, int amount = 20);

// Convert a raylib Color to a slightly darker shade
Color pres_darker(Color c, int amount = 20);

// Convert Color to RGBA hex string for display
std::string pres_colorHex(Color c);

// Check if the mouse is over a rectangle
bool pres_mouseOver(Rectangle rect);

// Clamp a float value
float pres_clamp(float value, float lo, float hi);

// Load fonts used by the application
void pres_loadFonts();

// Unload fonts on exit
void pres_unloadFonts();

// Externally accessible fonts
extern Font FONT_REGULAR;
extern Font FONT_BOLD;
extern Font FONT_MONO;
