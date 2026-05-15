#include "presentation.h"
#include "logic.h"
#include "data.h"
#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

 // ═══════════════════════════════════════════════════════════════
 //  Color palette definitions
 // ═══════════════════════════════════════════════════════════════

Color COL_BG = { 245, 247, 250, 255 };
Color COL_SURFACE = { 255, 255, 255, 255 };
Color COL_SURFACE2 = { 30,  41,  59, 255 };
Color COL_BORDER = { 226, 232, 240, 255 };
Color COL_PRIMARY = { 37,  99, 235, 255 };
Color COL_PRIMARY_DARK = { 29,  78, 216, 255 };
Color COL_ACCENT = { 16, 185, 129, 255 };
Color COL_TEXT = { 15,  23,  42, 255 };
Color COL_TEXT_MUTED = { 100, 116, 139, 255 };
Color COL_TEXT_LIGHT = { 241, 245, 249, 255 };

Color COL_GRADE[5] = {
    { 239,  68,  68, 255 },   // 0 – Red        (Fail)
    { 249, 115,  22, 255 },   // 1 – Orange      (Poor)
    { 234, 179,   8, 255 },   // 2 – Yellow      (Good)
    {  34, 197,  94, 255 },   // 3 – Green       (Very Good)
    {  22, 163,  74, 255 },   // 4 – Dark-green  (Excellent)
};

// ═══════════════════════════════════════════════════════════════
//  Font handles
// ═══════════════════════════════════════════════════════════════

Font FONT_REGULAR = { 0 };
Font FONT_BOLD = { 0 };
Font FONT_MONO = { 0 };

// ═══════════════════════════════════════════════════════════════
//  Internal helpers
// ═══════════════════════════════════════════════════════════════

static void drawText(const std::string& text, float x, float y,
    int fontSize, Color color, Font& font)
{
    if (font.texture.id == 0)
        DrawText(text.c_str(), (int)x, (int)y, fontSize, color);
    else
        DrawTextEx(font, text.c_str(), { x, y },
            static_cast<float>(fontSize), 1.0f, color);
}

static float measureText(const std::string& text, int fontSize, Font& font)
{
    if (font.texture.id == 0)
        return static_cast<float>(MeasureText(text.c_str(), fontSize));
    Vector2 size = MeasureTextEx(font, text.c_str(),
        static_cast<float>(fontSize), 1.0f);
    return size.x;
}

// ═══════════════════════════════════════════════════════════════
//  Font loading / unloading
// ═══════════════════════════════════════════════════════════════

void pres_loadFonts()
{
    FONT_REGULAR = GetFontDefault();
    FONT_BOLD = GetFontDefault();
    FONT_MONO = GetFontDefault();
}

void pres_unloadFonts() {}

// ═══════════════════════════════════════════════════════════════
//  Utility helpers
// ═══════════════════════════════════════════════════════════════

std::string pres_fmt2(double value)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

std::string pres_fmtGrade(double average)
{
    if (average <= 0.0) return "--";
    return pres_fmt2(average) + " - " + logic_gradeLabel(average);
}

Color pres_lighter(Color c, int a)
{
    return {
        (unsigned char)std::min(255, c.r + a),
        (unsigned char)std::min(255, c.g + a),
        (unsigned char)std::min(255, c.b + a),
        c.a
    };
}

Color pres_darker(Color c, int a)
{
    return {
        (unsigned char)std::max(0, c.r - a),
        (unsigned char)std::max(0, c.g - a),
        (unsigned char)std::max(0, c.b - a),
        c.a
    };
}

std::string pres_colorHex(Color c)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
    return buf;
}

bool pres_mouseOver(Rectangle rect)
{
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, rect);
}

float pres_clamp(float value, float lo, float hi)
{
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

// ═══════════════════════════════════════════════════════════════
//  Seed data
// ═══════════════════════════════════════════════════════════════

static void seedData(AppState& app)
{
    struct SeedStudent {
        const char* first; const char* last;
        const char* cls; Grade yr; Gender gen;
    };

    SeedStudent seeds[] = {
        { "Alexander", "Petrov",    "9A",  Grade::SECOND, Gender::MALE   },
        { "Maria",     "Ivanova",   "9A",  Grade::SECOND, Gender::FEMALE },
        { "George",    "Stoyanov",  "9A",  Grade::SECOND, Gender::MALE   },
        { "Simona",    "Dimova",    "9A",  Grade::SECOND, Gender::FEMALE },
        { "Nikolay",   "Todorov",   "9B",  Grade::SECOND, Gender::MALE   },
        { "Elena",     "Marinova",  "9B",  Grade::SECOND, Gender::FEMALE },
        { "Stefan",    "Kolev",     "9B",  Grade::SECOND, Gender::MALE   },
        { "Victoria",  "Nikolova",  "9B",  Grade::SECOND, Gender::FEMALE },
        { "Boiko",     "Atanasov",  "10A", Grade::THIRD,  Gender::MALE   },
        { "Anna",      "Hristova",  "10A", Grade::THIRD,  Gender::FEMALE },
        { "Lachezar",  "Genchev",   "10A", Grade::THIRD,  Gender::MALE   },
        { "Diana",     "Stefanova", "10B", Grade::THIRD,  Gender::FEMALE },
    };

    double gradeMatrix[][10] = {
        { 5.5, 4.5, 6.0, 5.0, 5.5, 4.0, 4.5, 5.0, 6.0, 5.0 },
        { 6.0, 5.5, 5.5, 6.0, 5.0, 5.5, 6.0, 5.5, 5.0, 6.0 },
        { 3.5, 4.0, 4.0, 3.5, 4.0, 3.0, 3.5, 4.0, 4.5, 4.0 },
        { 4.5, 5.0, 5.5, 4.0, 4.5, 5.0, 4.5, 5.0, 5.5, 4.5 },
        { 2.5, 3.0, 3.5, 2.5, 3.0, 2.0, 3.0, 3.5, 4.0, 3.0 },
        { 5.0, 5.5, 5.0, 5.5, 5.0, 4.5, 5.0, 5.5, 5.0, 5.5 },
        { 4.0, 4.5, 4.5, 4.0, 4.5, 3.5, 4.0, 4.5, 5.0, 4.0 },
        { 6.0, 6.0, 5.5, 6.0, 6.0, 5.5, 6.0, 6.0, 5.5, 6.0 },
        { 3.0, 3.5, 4.0, 3.0, 3.5, 2.5, 3.5, 4.0, 4.5, 3.5 },
        { 5.5, 5.0, 5.5, 5.0, 5.5, 5.0, 5.5, 5.0, 5.5, 5.5 },
        { 4.5, 4.0, 4.5, 4.5, 4.0, 4.5, 4.0, 4.5, 5.0, 4.5 },
        { 6.0, 5.5, 6.0, 5.5, 6.0, 5.5, 6.0, 5.5, 6.0, 5.5 },
    };

    auto allSubj = data_allSubjects();
    int n = sizeof(seeds) / sizeof(seeds[0]);

    for (int i = 0; i < n; ++i)
    {
        Student s;
        s.firstName = seeds[i].first;
        s.lastName = seeds[i].last;
        s.classLabel = seeds[i].cls;
        s.yearGroup = seeds[i].yr;
        s.gender = seeds[i].gen;
        s.isActive = true;
        s.egn = "";
        int id;
        logic_addStudent(app.store, s, id);

        for (int j = 0; j < (int)allSubj.size() && j < 10; ++j)
            logic_setGrade(app.store, id, allSubj[j], gradeMatrix[i][j]);
    }
}

// ═══════════════════════════════════════════════════════════════
//  Lifecycle
// ═══════════════════════════════════════════════════════════════

void pres_init(AppState& app)
{
    memset(app.searchBuf, 0, sizeof(app.searchBuf));
    memset(app.formFirst, 0, sizeof(app.formFirst));
    memset(app.formLast, 0, sizeof(app.formLast));
    memset(app.formEgn, 0, sizeof(app.formEgn));
    memset(app.formClass, 0, sizeof(app.formClass));
    memset(app.gradeValueBuf, 0, sizeof(app.gradeValueBuf));
    memset(app.gradeNoteBuf, 0, sizeof(app.gradeNoteBuf));
    memset(app.filterClass, 0, sizeof(app.filterClass));

    // Login state
    memset(app.loginUserBuf, 0, sizeof(app.loginUserBuf));
    memset(app.loginPassBuf, 0, sizeof(app.loginPassBuf));
    app.loggedInUser = nullptr;
    app.loginError = "";
    app.loginShowPass = false;
    app.loginShakeTimer = 0.0f;

    app.currentScreen = Screen::LOGIN;   // Always start at login
    app.activeModal = Modal::NONE;
    app.selectedStudentId = -1;
    app.tableScrollY = 0;
    app.tableHoverRow = -1;
    app.sortField = SortField::BY_LAST_NAME;
    app.sortOrder = SortOrder::ASCENDING;
    app.searchDirty = false;
    app.lastBinaryIdx = -1;
    app.searchUseBinary = false;
    app.gradeStudentId = -1;
    app.gradeSubject = Subject::MATHEMATICS;
    app.formGender = 0;
    app.formYearGroup = 2;
    app.editStudentId = -1;
    app.deleteTargetId = -1;
    app.demoSortField = SortField::BY_LAST_NAME;
    app.demoSortOrder = SortOrder::ASCENDING;
    app.demoUseBubble = true;
    app.demoSwapCount = 0;
    app.statsClassFilter = "";
    app.statsNeedsRefresh = true;
    app.notifTimer = 0.0f;
    app.focusedInput = 0;

    app.store = data_createStore();

    // Load or create user accounts
    app.userStore = data_createUserStore();
    data_loadUsers(app.userStore, USERS_FILE); // overwrite defaults if file exists

    LogicResult lr = logic_load(app.store, DATA_FILE);
    if (lr != LogicResult::OK || app.store.students.empty())
        seedData(app);

    app.displayedStudents = data_getAllActive(app.store);
    logic_quickSort(app.displayedStudents, app.sortField, app.sortOrder);

    app.currentStats = logic_schoolStats(app.store);
    app.statsNeedsRefresh = false;

    pres_loadFonts();
}

void pres_shutdown(AppState& app)
{
    logic_save(app.store, DATA_FILE);
    pres_unloadFonts();
}

// ═══════════════════════════════════════════════════════════════
//  UI Primitives
// ═══════════════════════════════════════════════════════════════

void pres_drawCard(Rectangle rect, Color bg, Color border, float radius)
{
    DrawRectangleRounded(rect, radius, 6, bg);
    DrawRectangleRoundedLines(rect, radius, 6, border);
}

void pres_drawLabel(Rectangle rect, const std::string& text,
    Color color, int fontSize, bool centred)
{
    float tw = measureText(text, fontSize, FONT_REGULAR);
    float x = centred ? rect.x + (rect.width - tw) / 2.0f : rect.x;
    float y = rect.y + (rect.height - fontSize) / 2.0f;
    drawText(text, x, y, fontSize, color, FONT_REGULAR);
}

bool pres_button(Rectangle rect, const std::string& label,
    Color bg, Color textColor, int fontSize)
{
    bool over = pres_mouseOver(rect);
    bool clicked = over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    Color fillColor = over ? pres_darker(bg, 15) : bg;

    DrawRectangleRounded(rect, 0.25f, 6, fillColor);
    float tw = measureText(label, fontSize, FONT_BOLD);
    float x = rect.x + (rect.width - tw) / 2.0f;
    float y = rect.y + (rect.height - fontSize) / 2.0f;
    drawText(label, x, y, fontSize, textColor, FONT_BOLD);

    return clicked;
}

bool pres_buttonGhost(Rectangle rect, const std::string& label, int fontSize)
{
    bool over = pres_mouseOver(rect);
    bool clicked = over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (over) DrawRectangleRounded(rect, 0.25f, 6, { 226, 232, 240, 255 });
    DrawRectangleRoundedLines(rect, 0.25f, 6, COL_BORDER);

    float tw = measureText(label, fontSize, FONT_REGULAR);
    float x = rect.x + (rect.width - tw) / 2.0f;
    float y = rect.y + (rect.height - fontSize) / 2.0f;
    drawText(label, x, y, fontSize, COL_TEXT, FONT_REGULAR);

    return clicked;
}

bool pres_textInput(Rectangle rect, char* buf, int bufSize,
    const std::string& placeholder, int& focusId,
    int thisId, int fontSize)
{
    bool over = pres_mouseOver(rect);
    bool focused = (focusId == thisId);

    if (over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        focusId = thisId;

    Color borderColor = focused ? COL_PRIMARY : COL_BORDER;

    DrawRectangleRounded(rect, 0.2f, 6, COL_SURFACE);
    DrawRectangleRoundedLines(rect, 0.2f, 6, borderColor);

    bool empty = (buf[0] == '\0');
    if (empty)
        drawText(placeholder, rect.x + 10,
            rect.y + (rect.height - fontSize) / 2.0f,
            fontSize, COL_TEXT_MUTED, FONT_REGULAR);
    else
        drawText(std::string(buf), rect.x + 10,
            rect.y + (rect.height - fontSize) / 2.0f,
            fontSize, COL_TEXT, FONT_REGULAR);

    if (focused)
    {
        float tw = empty ? 0.0f : measureText(std::string(buf), fontSize, FONT_REGULAR);
        float cx = rect.x + 10 + tw + 2;
        float cy = rect.y + 6;
        if (((int)(GetTime() * 2)) % 2 == 0)
            DrawLine((int)cx, (int)cy, (int)cx,
                (int)(rect.y + rect.height - 6), COL_PRIMARY);
    }

    bool enter = false;
    if (focused)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            int len = (int)strlen(buf);
            if (key >= 32 && len < bufSize - 1)
            {
                buf[len] = static_cast<char>(key);
                buf[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            int len = (int)strlen(buf);
            if (len > 0) buf[len - 1] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER)) enter = true;
    }

    return enter;
}

void pres_gradeBadge(float x, float y, double grade, int fontSize)
{
    int   ci = logic_gradeColorIndex(grade);
    Color bg = (ci < 0) ? COL_TEXT_MUTED : COL_GRADE[ci];

    std::string text = (grade <= 0.0) ? "--" : pres_fmt2(grade);
    float tw = measureText(text, fontSize, FONT_BOLD) + 16.0f;
    float h = static_cast<float>(fontSize) + 8.0f;

    Rectangle rect = { x, y, tw, h };
    DrawRectangleRounded(rect, 0.5f, 6, bg);
    float tx = x + (tw - measureText(text, fontSize, FONT_BOLD)) / 2.0f;
    float ty = y + (h - fontSize) / 2.0f;
    drawText(text, tx, ty, fontSize, WHITE, FONT_BOLD);
}

void pres_progressBar(Rectangle rect, float value, Color fill, Color bg)
{
    DrawRectangleRounded(rect, 0.5f, 6, bg);
    if (value > 0.0f)
    {
        Rectangle bar = { rect.x, rect.y,
                          rect.width * pres_clamp(value, 0.0f, 1.0f),
                          rect.height };
        DrawRectangleRounded(bar, 0.5f, 6, fill);
    }
}

void pres_notify(AppState& app, const std::string& message, Color color)
{
    app.notifMessage = message;
    app.notifColor = color;
    app.notifTimer = 2.5f;
}

void pres_statCard(Rectangle rect, const std::string& valueStr,
    const std::string& label, Color accent)
{
    pres_drawCard(rect, COL_SURFACE, COL_BORDER, 0.15f);
    DrawRectangleRounded({ rect.x, rect.y, 5, rect.height }, 0.3f, 4, accent);

    float px = rect.x + 20.0f;
    float py = rect.y + 14.0f;
    drawText(valueStr, px, py, 28, COL_TEXT, FONT_BOLD);
    drawText(label, px, py + 36, 13, COL_TEXT_MUTED, FONT_REGULAR);
}

void pres_drawGradeLegend(float x, float y)
{
    const char* labels[] = { "Fail", "Poor", "Good", "Very Good", "Excellent" };
    const char* ranges[] = { "<3.0", "3.0-3.5", "3.5-4.5", "4.5-5.5", ">=5.5" };

    for (int i = 0; i < 5; ++i)
    {
        float px = x + i * 120.0f;
        DrawRectangleRounded({ px, y, 14, 14 }, 0.4f, 4, COL_GRADE[i]);
        drawText(labels[i], px + 18, y, 12, COL_TEXT, FONT_REGULAR);
        drawText(ranges[i], px + 18, y + 14, 10, COL_TEXT_MUTED, FONT_REGULAR);
    }
}

void pres_separator(float x, float y, float width, Color color)
{
    DrawLine((int)x, (int)y, (int)(x + width), (int)y, color);
}

void pres_sectionHeader(float x, float y, float width,
    const std::string& title, Color color)
{
    drawText(title, x, y, 13, COL_TEXT_MUTED, FONT_REGULAR);
    pres_separator(x, y + 18, width, COL_BORDER);
}

bool pres_cycleSelector(Rectangle rect, const std::string& label,
    const std::vector<std::string>& options,
    int& selectedIndex)
{
    bool changed = false;
    bool clicked = pres_mouseOver(rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (clicked)
    {
        selectedIndex = (selectedIndex + 1) % static_cast<int>(options.size());
        changed = true;
    }

    Color bg = pres_mouseOver(rect)
        ? pres_lighter(COL_SURFACE2, 10) : COL_SURFACE2;
    DrawRectangleRounded(rect, 0.25f, 6, bg);

    std::string text = label + ": " + options[selectedIndex];
    float tw = measureText(text, 13, FONT_REGULAR);
    float tx = rect.x + (rect.width - tw) / 2.0f;
    float ty = rect.y + (rect.height - 13) / 2.0f;
    drawText(text, tx, ty, 13, COL_TEXT_LIGHT, FONT_REGULAR);

    return changed;
}

// ═══════════════════════════════════════════════════════════════
//  Login Screen
// ═══════════════════════════════════════════════════════════════

void pres_drawLoginScreen(AppState& app)
{
    // Dark gradient background (drawn as two rectangles for compatibility)
    Color bgTop = { 15,  23,  42,  255 };
    Color bgBottom = { 30,  41,  59,  255 };
    DrawRectangleGradientV(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, bgTop, bgBottom);

    // Decorative soft glow circles (drawn as plain translucent circles)
    DrawCircle(WINDOW_WIDTH - 200, 100, 280, Color{ 37,  99, 235, 18 });
    DrawCircle(WINDOW_WIDTH - 200, 100, 180, Color{ 37,  99, 235, 12 });
    DrawCircle(200, WINDOW_HEIGHT - 100, 240, Color{ 16, 185, 129, 15 });
    DrawCircle(200, WINDOW_HEIGHT - 100, 140, Color{ 16, 185, 129, 10 });

    // Card shake offset on bad login
    float shakeX = 0.0f;
    if (app.loginShakeTimer > 0.0f)
    {
        app.loginShakeTimer -= GetFrameTime();
        shakeX = sinf(app.loginShakeTimer * 40.0f) * 6.0f;
    }

    // Login card
    float cardW = 420.0f;
    float cardH = 480.0f;
    float cardX = (WINDOW_WIDTH - cardW) / 2.0f + shakeX;
    float cardY = (WINDOW_HEIGHT - cardH) / 2.0f;

    DrawRectangleRounded({ cardX, cardY, cardW, cardH }, 0.08f, 8,
        { 255, 255, 255, 255 });
    DrawRectangleRoundedLines({ cardX, cardY, cardW, cardH }, 0.08f, 8,
        { 226, 232, 240, 255 });

    // Top accent bar
    DrawRectangleRounded({ cardX, cardY, cardW, 6 }, 0.08f, 6,
        COL_PRIMARY);

    float px = cardX + 40.0f;
    float py = cardY + 36.0f;

    // Logo / title
    float logoW = MeasureText("GradeBook", 32);
    DrawText("GradeBook",
        (int)(cardX + (cardW - logoW) / 2.0f),
        (int)py, 32, { 37, 99, 235, 255 });
    py += 44;

    float subW = MeasureText("Student Grade Management System", 13);
    DrawText("Student Grade Management System",
        (int)(cardX + (cardW - subW) / 2.0f),
        (int)py, 13, { 100, 116, 139, 255 });
    py += 44;

    // Divider
    DrawLine((int)px, (int)py, (int)(cardX + cardW - 40), (int)py,
        { 226, 232, 240, 255 });
    py += 20;

    // Welcome text
    DrawText("Welcome back", (int)px, (int)py, 20, { 15, 23, 42, 255 });
    py += 28;
    DrawText("Sign in to your account", (int)px, (int)py, 13,
        { 100, 116, 139, 255 });
    py += 36;

    // Username field
    DrawText("Username", (int)px, (int)py, 13, { 71, 85, 105, 255 });
    py += 18;
    Rectangle userRect = { px, py, cardW - 80, 42 };
    bool userFocused = (app.focusedInput == 500);
    DrawRectangleRounded(userRect, 0.2f, 6, { 248, 250, 252, 255 });
    DrawRectangleRoundedLines(userRect, 0.2f, 6,
        userFocused ? COL_PRIMARY : COL_BORDER);
    if (pres_mouseOver(userRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        app.focusedInput = 500;

    if (app.loginUserBuf[0] == '\0')
        DrawText("e.g. admin", (int)px + 12, (int)py + 13, 14,
            { 148, 163, 184, 255 });
    else
        DrawText(app.loginUserBuf, (int)px + 12, (int)py + 13, 14,
            { 15, 23, 42, 255 });

    // Cursor blink in username
    if (userFocused && ((int)(GetTime() * 2)) % 2 == 0)
    {
        int tw = MeasureText(app.loginUserBuf, 14);
        DrawLine((int)px + 14 + tw, (int)py + 8,
            (int)px + 14 + tw, (int)py + 34, COL_PRIMARY);
    }
    py += 52;

    // Password field
    DrawText("Password", (int)px, (int)py, 13, { 71, 85, 105, 255 });
    py += 18;
    Rectangle passRect = { px, py, cardW - 80, 42 };
    bool passFocused = (app.focusedInput == 501);
    DrawRectangleRounded(passRect, 0.2f, 6, { 248, 250, 252, 255 });
    DrawRectangleRoundedLines(passRect, 0.2f, 6,
        passFocused ? COL_PRIMARY : COL_BORDER);
    if (pres_mouseOver(passRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        app.focusedInput = 501;

    // Show masked or plain password
    if (app.loginPassBuf[0] == '\0')
        DrawText("Password", (int)px + 12, (int)py + 13, 14,
            { 148, 163, 184, 255 });
    else
    {
        if (app.loginShowPass)
            DrawText(app.loginPassBuf, (int)px + 12, (int)py + 13, 14,
                { 15, 23, 42, 255 });
        else
        {
            // Draw dots for each character
            int len = (int)strlen(app.loginPassBuf);
            std::string dots(len, '*');
            DrawText(dots.c_str(), (int)px + 12, (int)py + 13, 14,
                { 15, 23, 42, 255 });
        }
    }

    // Cursor blink in password
    if (passFocused && ((int)(GetTime() * 2)) % 2 == 0)
    {
        int len = (int)strlen(app.loginPassBuf);
        std::string display = app.loginShowPass
            ? std::string(app.loginPassBuf)
            : std::string(len, '*');
        int tw = MeasureText(display.c_str(), 14);
        DrawLine((int)px + 14 + tw, (int)py + 8,
            (int)px + 14 + tw, (int)py + 34, COL_PRIMARY);
    }

    // Show/hide password toggle button
    Rectangle showRect = { cardX + cardW - 74, py + 8, 26, 26 };
    bool showHover = pres_mouseOver(showRect);
    DrawRectangleRounded(showRect, 0.3f, 4,
        showHover ? COL_BG : BLANK);
    DrawText(app.loginShowPass ? "***" : "abc",
        (int)showRect.x + 3, (int)showRect.y + 6, 11,
        { 100, 116, 139, 255 });
    if (showHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        app.loginShowPass = !app.loginShowPass;

    py += 52;

    // Error message
    if (!app.loginError.empty())
    {
        DrawRectangleRounded({ px, py, cardW - 80, 34 }, 0.2f, 6,
            { 254, 226, 226, 255 });
        DrawRectangleRoundedLines({ px, py, cardW - 80, 34 }, 0.2f, 6,
            { 252, 165, 165, 255 });
        float ew = (float)MeasureText(app.loginError.c_str(), 12);
        DrawText(app.loginError.c_str(),
            (int)(px + (cardW - 80 - ew) / 2.0f),
            (int)py + 11, 12, { 185, 28, 28, 255 });
        py += 44;
    }
    else
    {
        py += 8;
    }

    // Sign in button
    Rectangle btnRect = { px, py, cardW - 80, 44 };
    bool btnHover = pres_mouseOver(btnRect);
    DrawRectangleRounded(btnRect, 0.25f, 6,
        btnHover ? pres_darker(COL_PRIMARY, 15) : COL_PRIMARY);
    float bw = (float)MeasureText("Sign In", 16);
    DrawText("Sign In",
        (int)(px + (cardW - 80 - bw) / 2.0f),
        (int)py + 14, 16, WHITE);
    py += 54;

    // Hint text showing default credentials
    DrawText("Default:  admin / admin123", (int)px, (int)py, 11,
        { 148, 163, 184, 255 });

    // ── Handle keyboard input ────────────────────────────────
    auto handleInput = [&](char* buf, int bufSize) {
        int key = GetCharPressed();
        while (key > 0)
        {
            int len = (int)strlen(buf);
            if (key >= 32 && len < bufSize - 1)
            {
                buf[len] = static_cast<char>(key);
                buf[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            int len = (int)strlen(buf);
            if (len > 0) buf[len - 1] = '\0';
        }
        };

    if (app.focusedInput == 500) handleInput(app.loginUserBuf, INPUT_BUF);
    if (app.focusedInput == 501) handleInput(app.loginPassBuf, INPUT_BUF);

    // Tab key switches between fields
    if (IsKeyPressed(KEY_TAB))
        app.focusedInput = (app.focusedInput == 500) ? 501 : 500;

    // Attempt login on button click or Enter key
    bool tryLogin = (btnHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        || IsKeyPressed(KEY_ENTER);

    if (tryLogin)
    {
        const User* user = logic_login(app.userStore,
            std::string(app.loginUserBuf),
            std::string(app.loginPassBuf));
        if (user)
        {
            // Success - enter the app
            app.loggedInUser = user;
            app.loginError = "";
            app.currentScreen = Screen::DASHBOARD;
            app.focusedInput = 0;
            memset(app.loginPassBuf, 0, sizeof(app.loginPassBuf));
        }
        else
        {
            // Bad credentials - show error and shake the card
            app.loginError = "Invalid username or password.";
            app.loginShakeTimer = 0.4f;
            memset(app.loginPassBuf, 0, sizeof(app.loginPassBuf));
        }
    }
}

// ═══════════════════════════════════════════════════════════════
//  Top Bar
// ═══════════════════════════════════════════════════════════════

void pres_drawTopBar(AppState& app)
{
    DrawRectangle(0, 0, WINDOW_WIDTH, TOPBAR_HEIGHT, COL_SURFACE);
    DrawLine(0, TOPBAR_HEIGHT, WINDOW_WIDTH, TOPBAR_HEIGHT, COL_BORDER);

    drawText("GradeBook", SIDEBAR_WIDTH + 16.0f, 18, 20, COL_PRIMARY, FONT_BOLD);

    const char* subtitle = "";
    switch (app.currentScreen)
    {
    case Screen::DASHBOARD: subtitle = "Dashboard";  break;
    case Screen::STUDENTS:  subtitle = "Students";   break;
    case Screen::GRADES:    subtitle = "Grades";     break;
    case Screen::SEARCH:    subtitle = "Search";     break;
    case Screen::SORT_DEMO: subtitle = "Sort Demo";  break;
    case Screen::STATS:     subtitle = "Statistics"; break;
    case Screen::ABOUT:     subtitle = "About";      break;
    default:                subtitle = "";           break;
    }
    drawText(subtitle, SIDEBAR_WIDTH + 120.0f, 22, 14, COL_TEXT_MUTED, FONT_REGULAR);

    // Logged-in user badge + logout button (right side, before Save)
    if (app.loggedInUser)
    {
        std::string userInfo = app.loggedInUser->displayName
            + "  [" + data_roleName(app.loggedInUser->role) + "]";
        drawText(userInfo, WINDOW_WIDTH - 330.0f, 10, 11, COL_TEXT_MUTED, FONT_REGULAR);

        if (pres_button({ WINDOW_WIDTH - 330.0f, 26, 76, 24 },
            "Log out", { 100, 116, 139, 255 }, WHITE, 11))
        {
            app.loggedInUser = nullptr;
            app.currentScreen = Screen::LOGIN;
            app.focusedInput = 500;
            memset(app.loginUserBuf, 0, sizeof(app.loginUserBuf));
            memset(app.loginPassBuf, 0, sizeof(app.loginPassBuf));
            app.loginError = "";
        }
    }

    if (pres_button({ WINDOW_WIDTH - 110.0f, 14, 94, 32 },
        "Save", COL_ACCENT, WHITE, 14))
    {
        LogicResult lr = logic_save(app.store, DATA_FILE);
        if (lr == LogicResult::OK)
            pres_notify(app, "Data saved successfully.");
        else
            pres_notify(app, "Error saving data!", { 239, 68, 68, 255 });
    }

    if (app.notifTimer > 0.0f)
    {
        app.notifTimer -= GetFrameTime();
        float alpha = pres_clamp(app.notifTimer * 2.0f, 0.0f, 1.0f);
        Color nc = app.notifColor;
        nc.a = static_cast<unsigned char>(255 * alpha);

        Rectangle nr = { WINDOW_WIDTH - 360.0f, TOPBAR_HEIGHT + 8, 348, 38 };
        DrawRectangleRounded(nr, 0.3f, 6, nc);
        float tw = measureText(app.notifMessage, 13, FONT_REGULAR);
        drawText(app.notifMessage,
            nr.x + (nr.width - tw) / 2.0f, nr.y + 12,
            13, WHITE, FONT_REGULAR);
    }
}

// ═══════════════════════════════════════════════════════════════
//  Sidebar
// ═══════════════════════════════════════════════════════════════

void pres_drawSidebar(AppState& app)
{
    DrawRectangle(0, 0, SIDEBAR_WIDTH, WINDOW_HEIGHT, COL_SURFACE2);
    DrawRectangle(0, 0, SIDEBAR_WIDTH, TOPBAR_HEIGHT,
        pres_darker(COL_SURFACE2, 8));
    drawText("GradeBook", 16, 18, 18, COL_TEXT_LIGHT, FONT_BOLD);

    struct MenuItem { const char* label; Screen screen; };
    MenuItem items[] = {
        { "Dashboard",  Screen::DASHBOARD  },
        { "Students",   Screen::STUDENTS   },
        { "Grades",     Screen::GRADES     },
        { "Search",     Screen::SEARCH     },
        { "Sort Demo",  Screen::SORT_DEMO  },
        { "Statistics", Screen::STATS      },
        { "About",      Screen::ABOUT      },
    };

    float itemH = 46.0f;
    float startY = TOPBAR_HEIGHT + 16.0f;

    for (int i = 0; i < 7; ++i)
    {
        Rectangle r = { 8.0f, startY + i * itemH,
                        SIDEBAR_WIDTH - 16.0f, itemH - 4.0f };
        bool active = (app.currentScreen == items[i].screen);
        bool over = pres_mouseOver(r);

        Color itemBg = active ? COL_PRIMARY :
            (over ? pres_lighter(COL_SURFACE2, 12) : BLANK);

        if (itemBg.a > 0) DrawRectangleRounded(r, 0.2f, 6, itemBg);
        if (active) DrawRectangle(8, (int)r.y, 4, (int)r.height, COL_ACCENT);

        drawText(items[i].label, 32, r.y + 14, 14, COL_TEXT_LIGHT, FONT_REGULAR);

        if (over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            app.currentScreen = items[i].screen;
    }

    int activeCount = (int)data_getAllActive(app.store).size();
    std::string countStr = "Students: " + std::to_string(activeCount);
    drawText(countStr, 16, WINDOW_HEIGHT - 32, 12, COL_TEXT_MUTED, FONT_REGULAR);
}

// ═══════════════════════════════════════════════════════════════
//  Dashboard
// ═══════════════════════════════════════════════════════════════

void pres_drawDashboard(AppState& app)
{
    float cx = SIDEBAR_WIDTH + 16.0f;
    float cy = TOPBAR_HEIGHT + 20.0f;
    float cw = WINDOW_WIDTH - SIDEBAR_WIDTH - 32.0f;

    drawText("Welcome to GradeBook", cx, cy, 22, COL_TEXT, FONT_BOLD);
    drawText("Student Grade Management System",
        cx, cy + 30, 14, COL_TEXT_MUTED, FONT_REGULAR);
    cy += 72;

    GroupStats gs = logic_schoolStats(app.store);
    float cardW = (cw - 48.0f) / 4.0f;
    float cardH = 90.0f;

    pres_statCard({ cx,                  cy, cardW, cardH },
        std::to_string(gs.totalStudents), "Students", COL_PRIMARY);
    pres_statCard({ cx + cardW + 16,     cy, cardW, cardH },
        pres_fmt2(gs.overallAverage), "Avg. Grade", COL_ACCENT);
    pres_statCard({ cx + 2 * (cardW + 16), cy, cardW, cardH },
        std::to_string(gs.excellentCount), "Excellent", { 234, 179, 8, 255 });
    pres_statCard({ cx + 3 * (cardW + 16), cy, cardW, cardH },
        std::to_string(gs.totalFailing), "Failing", COL_GRADE[0]);

    cy += cardH + 24;

    drawText("Top Students", cx, cy, 16, COL_TEXT, FONT_BOLD);
    cy += 28;

    std::vector<Student*> ranking = logic_buildRanking(app.store);
    int showCount = std::min(8, (int)ranking.size());

    DrawRectangleRounded({ cx, cy, cw, 32 }, 0.15f, 6, COL_SURFACE2);
    drawText("Student", cx + 12, cy + 9, 13, COL_TEXT_LIGHT, FONT_BOLD);
    drawText("Class", cx + 260, cy + 9, 13, COL_TEXT_LIGHT, FONT_BOLD);
    drawText("Average", cx + 360, cy + 9, 13, COL_TEXT_LIGHT, FONT_BOLD);
    drawText("Level", cx + 520, cy + 9, 13, COL_TEXT_LIGHT, FONT_BOLD);
    cy += 32;

    for (int i = 0; i < showCount; ++i)
    {
        Student* s = ranking[i];
        double   avg = logic_studentAverage(*s);
        Color    rowBg = (i % 2 == 0) ? COL_SURFACE : COL_BG;
        DrawRectangle((int)cx, (int)cy, (int)cw, ROW_HEIGHT - 4, rowBg);

        drawText(std::to_string(i + 1) + ".", cx + 8, cy + 12, 13, COL_TEXT_MUTED, FONT_REGULAR);
        std::string fullName = s->firstName + " " + s->lastName;
        drawText(fullName, cx + 36, cy + 12, 13, COL_TEXT, FONT_REGULAR);
        drawText(s->classLabel, cx + 260, cy + 12, 13, COL_TEXT_MUTED, FONT_REGULAR);
        pres_gradeBadge(cx + 360, cy + 8, avg, 13);
        drawText(logic_gradeLabel(avg), cx + 520, cy + 12, 13, COL_TEXT_MUTED, FONT_REGULAR);

        cy += ROW_HEIGHT - 4;
    }

    cy += 24;
    drawText("Average by Subject", cx, cy, 16, COL_TEXT, FONT_BOLD);
    cy += 28;

    for (const SubjectStats& ss : gs.bySubject)
    {
        drawText(data_subjectName(ss.subject), cx, cy + 4, 13, COL_TEXT, FONT_REGULAR);

        float barX = cx + 200.0f;
        float barW = cw - 280.0f;
        float fill = (float)((ss.average - 2.0) / 4.0);
        int   ci = logic_gradeColorIndex(ss.average);
        Color fillC = (ci < 0) ? COL_TEXT_MUTED : COL_GRADE[ci];

        pres_progressBar({ barX, cy + 4, barW, 18 }, fill, fillC, COL_BORDER);
        drawText(pres_fmt2(ss.average), barX + barW + 8, cy + 4,
            13, COL_TEXT, FONT_REGULAR);
        cy += 26;
        if (cy > WINDOW_HEIGHT - 32) break;
    }
}

// ═══════════════════════════════════════════════════════════════
//  Students Screen
// ═══════════════════════════════════════════════════════════════

void pres_drawStudentsScreen(AppState& app)
{
    float cx = SIDEBAR_WIDTH + 16.0f;
    float cy = TOPBAR_HEIGHT + 16.0f;
    float cw = WINDOW_WIDTH - SIDEBAR_WIDTH - 32.0f;

    drawText("Students", cx, cy, 20, COL_TEXT, FONT_BOLD);
    cy += 32;

    pres_textInput({ cx, cy, 180, 34 }, app.filterClass, INPUT_BUF,
        "Filter by class...", app.focusedInput, 100);

    std::vector<std::string> fieldOpts = { "Last Name", "Average", "Class", "ID" };
    std::vector<std::string> orderOpts = { "Ascending", "Descending" };

    int fi = static_cast<int>(app.sortField);
    int oi = static_cast<int>(app.sortOrder);

    bool fieldChanged = pres_cycleSelector({ cx + 192, cy, 160, 34 },
        "Sort", fieldOpts, fi);
    bool orderChanged = pres_cycleSelector({ cx + 360, cy, 160, 34 },
        "Order", orderOpts, oi);

    if (fieldChanged || orderChanged ||
        IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_F5))
    {
        app.sortField = static_cast<SortField>(fi);
        app.sortOrder = static_cast<SortOrder>(oi);

        std::string filter(app.filterClass);
        if (filter.empty())
            app.displayedStudents = data_getAllActive(app.store);
        else
            app.displayedStudents = data_findByClass(app.store, filter);

        logic_quickSort(app.displayedStudents, app.sortField, app.sortOrder);
    }

    if (logic_canEdit(app.loggedInUser) &&
        pres_button({ cx + cw - 152, cy, 152, 34 },
            "+ New Student", COL_PRIMARY, WHITE, 14))
    {
        memset(app.formFirst, 0, sizeof(app.formFirst));
        memset(app.formLast, 0, sizeof(app.formLast));
        memset(app.formEgn, 0, sizeof(app.formEgn));
        memset(app.formClass, 0, sizeof(app.formClass));
        app.formGender = 0;
        app.formYearGroup = 2;
        app.editStudentId = -1;
        app.formError = "";
        app.activeModal = Modal::ADD_STUDENT;
    }

    cy += 46;

    DrawRectangleRounded({ cx, cy, cw, 34 }, 0.15f, 6, COL_SURFACE2);
    float col[] = { cx + 10, cx + 50, cx + 200, cx + 340, cx + 410, cx + 520, cx + 650 };
    const char* hdrs[] = { "#", "ID", "Student", "Class", "Gender", "Average", "Actions" };
    for (int i = 0; i < 7; ++i)
        drawText(hdrs[i], col[i], cy + 10, 12, COL_TEXT_LIGHT, FONT_BOLD);
    cy += 34;

    float tableH = WINDOW_HEIGHT - cy - 8.0f;
    int   visRows = (int)(tableH / ROW_HEIGHT);
    int   total = (int)app.displayedStudents.size();

    if (GetMouseWheelMove() != 0 && !app.displayedStudents.empty())
    {
        app.tableScrollY -= (int)(GetMouseWheelMove() * 3);
        app.tableScrollY = std::max(0, std::min(app.tableScrollY,
            std::max(0, total - visRows)));
    }

    BeginScissorMode((int)cx, (int)cy, (int)cw, (int)tableH);

    for (int i = app.tableScrollY;
        i < total && (i - app.tableScrollY) < visRows + 1; ++i)
    {
        Student* s = app.displayedStudents[i];
        double   avg = logic_studentAverage(*s);
        float    ry = cy + (i - app.tableScrollY) * (float)ROW_HEIGHT;
        bool     sel = (s->id == app.selectedStudentId);
        bool     hov = pres_mouseOver({ cx, ry, cw, (float)ROW_HEIGHT });

        Color rowBg = sel ? Color{ 219, 234, 254, 255 } :
            hov ? COL_BG : ((i % 2 == 0) ? COL_SURFACE : COL_BG);
        DrawRectangle((int)cx, (int)ry, (int)cw, ROW_HEIGHT - 1, rowBg);

        drawText(std::to_string(i + 1), col[0], ry + 13, 12, COL_TEXT_MUTED, FONT_REGULAR);
        drawText(std::to_string(s->id), col[1], ry + 13, 12, COL_TEXT_MUTED, FONT_REGULAR);

        std::string fullName = s->firstName + " " + s->lastName;
        drawText(fullName, col[2], ry + 13, 13, COL_TEXT, FONT_REGULAR);
        drawText(s->classLabel, col[3], ry + 13, 13, COL_TEXT_MUTED, FONT_REGULAR);
        drawText(data_genderName(s->gender), col[4], ry + 13, 12,
            COL_TEXT_MUTED, FONT_REGULAR);
        pres_gradeBadge(col[5], ry + 9, avg, 12);

        if (pres_button({ col[6], ry + 8, 54, 26 }, "Detail",
            COL_PRIMARY, WHITE, 11))
        {
            app.selectedStudentId = s->id;
            app.activeModal = Modal::STUDENT_DETAIL;
        }
        if (logic_canEdit(app.loggedInUser) &&
            pres_button({ col[6] + 60, ry + 8, 44, 26 }, "Edit",
                { 100, 116, 139, 255 }, WHITE, 11))
        {
            app.editStudentId = s->id;
            strncpy(app.formFirst, s->firstName.c_str(), INPUT_BUF - 1);
            strncpy(app.formLast, s->lastName.c_str(), INPUT_BUF - 1);
            strncpy(app.formClass, s->classLabel.c_str(), INPUT_BUF - 1);
            app.formGender = static_cast<int>(s->gender);
            app.formYearGroup = static_cast<int>(s->yearGroup);
            app.formError = "";
            app.activeModal = Modal::EDIT_STUDENT;
        }
        if (logic_canDelete(app.loggedInUser) &&
            pres_button({ col[6] + 110, ry + 8, 40, 26 }, "Del.",
                COL_GRADE[0], WHITE, 11))
        {
            app.deleteTargetId = s->id;
            app.activeModal = Modal::DELETE_CONFIRM;
        }

        if (hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            app.selectedStudentId = s->id;
    }

    EndScissorMode();

    if (total > visRows)
    {
        float trackH = tableH;
        float thumbH = trackH * visRows / total;
        float thumbY = cy + trackH * app.tableScrollY / total;
        DrawRectangle(WINDOW_WIDTH - 10, (int)cy, 6, (int)trackH, COL_BORDER);
        DrawRectangle(WINDOW_WIDTH - 10, (int)thumbY, 6, (int)thumbH, COL_TEXT_MUTED);
    }

    std::string info = "Showing: " + std::to_string(total) + " students";
    drawText(info, cx, WINDOW_HEIGHT - 20, 12, COL_TEXT_MUTED, FONT_REGULAR);
}

// ═══════════════════════════════════════════════════════════════
//  Grades Screen
// ═══════════════════════════════════════════════════════════════

void pres_drawGradesScreen(AppState& app)
{
    float cx = SIDEBAR_WIDTH + 16.0f;
    float cy = TOPBAR_HEIGHT + 16.0f;
    float cw = WINDOW_WIDTH - SIDEBAR_WIDTH - 32.0f;

    drawText("Grades", cx, cy, 20, COL_TEXT, FONT_BOLD);
    cy += 36;

    drawText("Select a student from the list to manage their grades.",
        cx, cy, 13, COL_TEXT_MUTED, FONT_REGULAR);
    cy += 26;

    std::vector<Student*> all = data_getAllActive(app.store);
    logic_quickSort(all, SortField::BY_LAST_NAME, SortOrder::ASCENDING);

    float listW = 260.0f;
    float listH = WINDOW_HEIGHT - cy - 16.0f;
    pres_drawCard({ cx, cy, listW, listH }, COL_SURFACE, COL_BORDER, 0.1f);

    int visRows = (int)(listH / 40);

    for (int i = 0; i < (int)all.size() && i < visRows; ++i)
    {
        Student* s = all[i];
        bool     sel = (s->id == app.gradeStudentId);
        Rectangle r = { cx + 4, cy + 4 + i * 40.0f, listW - 8, 36 };
        bool     hov = pres_mouseOver(r);

        Color bg = sel ? Color{ 219, 234, 254, 255 } :
            hov ? COL_BG : BLANK;
        if (bg.a > 0) DrawRectangleRounded(r, 0.2f, 6, bg);

        std::string name = s->firstName + " " + s->lastName;
        drawText(name, r.x + 8, r.y + 10, 13, COL_TEXT, FONT_REGULAR);
        drawText(s->classLabel, r.x + listW - 44, r.y + 12,
            11, COL_TEXT_MUTED, FONT_REGULAR);

        if (hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            app.gradeStudentId = s->id;
    }

    float gx = cx + listW + 16;
    float gw = cw - listW - 16;

    if (app.gradeStudentId == -1)
    {
        drawText("<-- Select a student", gx + 20, cy + 20, 14,
            COL_TEXT_MUTED, FONT_REGULAR);
        return;
    }

    Student* sel = data_findById(app.store, app.gradeStudentId);
    if (!sel) return;

    std::string hdr = sel->firstName + " " + sel->lastName
        + "  (" + sel->classLabel + ")";
    drawText(hdr, gx, cy, 16, COL_TEXT, FONT_BOLD);

    double avg = logic_studentAverage(*sel);
    drawText("Average: " + pres_fmtGrade(avg), gx, cy + 22,
        13, COL_TEXT_MUTED, FONT_REGULAR);
    cy += 52;

    auto allSubj = data_allSubjects();
    int  cols = 2;
    float cellW = (gw - 8) / cols;
    float cellH = 60.0f;

    for (int i = 0; i < (int)allSubj.size(); ++i)
    {
        Subject subj = allSubj[i];
        double  g = data_getGrade(*sel, subj);
        int     c = i % cols;
        int     row = i / cols;

        float rx = gx + c * (cellW + 8);
        float ry = cy + row * (cellH + 8);

        pres_drawCard({ rx, ry, cellW, cellH }, COL_SURFACE, COL_BORDER, 0.15f);
        drawText(data_subjectName(subj), rx + 10, ry + 8, 13, COL_TEXT, FONT_REGULAR);
        pres_gradeBadge(rx + 10, ry + 30, g, 13);

        if (pres_button({ rx + cellW - 66, ry + 17, 60, 26 },
            g > 0 ? "Change" : "+ Add",
            g > 0 ? COL_PRIMARY : COL_ACCENT, WHITE, 11))
        {
            app.gradeSubject = subj;
            if (g > 0)
                snprintf(app.gradeValueBuf, sizeof(app.gradeValueBuf), "%.2f", g);
            else
                app.gradeValueBuf[0] = '\0';
            app.gradeNoteBuf[0] = '\0';
            app.gradeError = "";
            app.activeModal = Modal::ADD_GRADE;
        }
    }
}

// ═══════════════════════════════════════════════════════════════
//  Search Screen
// ═══════════════════════════════════════════════════════════════

void pres_drawSearchScreen(AppState& app)
{
    float cx = SIDEBAR_WIDTH + 16.0f;
    float cy = TOPBAR_HEIGHT + 16.0f;
    float cw = WINDOW_WIDTH - SIDEBAR_WIDTH - 32.0f;

    drawText("Search Students", cx, cy, 20, COL_TEXT, FONT_BOLD);
    cy += 36;

    bool enter = pres_textInput({ cx, cy, cw - 220, 42 }, app.searchBuf, INPUT_BUF,
        "Enter last name...", app.focusedInput, 200);

    std::vector<std::string> methods = { "Linear", "Binary" };
    int mi = app.searchUseBinary ? 1 : 0;
    if (pres_cycleSelector({ cx + cw - 214, cy, 210, 42 },
        "Method", methods, mi))
        app.searchUseBinary = (mi == 1);

    cy += 52;

    auto runSearch = [&]() {
        std::string query(app.searchBuf);
        if (!app.searchUseBinary)
        {
            app.searchResults = logic_linearSearchByName(app.store, query);
            app.lastBinaryIdx = -1;
        }
        else
        {
            app.searchResults = data_getAllActive(app.store);
            logic_bubbleSort(app.searchResults,
                SortField::BY_LAST_NAME, SortOrder::ASCENDING);
            app.lastBinaryIdx = logic_binarySearchByName(app.searchResults, query);
        }
        };

    if (enter) runSearch();

    if (pres_button({ cx, cy, 100, 36 }, "Search", COL_PRIMARY, WHITE, 14))
        runSearch();

    cy += 52;

    std::string methDesc = app.searchUseBinary
        ? "Binary Search: Works on a sorted array only. O(log n). Faster for large datasets."
        : "Linear Search: Checks every record sequentially. O(n). Works on unsorted arrays.";

    pres_drawCard({ cx, cy, cw, 44 }, { 239, 246, 255, 255 },
        { 147, 197, 253, 255 }, 0.1f);
    drawText(methDesc, cx + 12, cy + 12, 13, { 30, 64, 175, 255 }, FONT_REGULAR);
    cy += 56;

    if (app.searchResults.empty() && strlen(app.searchBuf) == 0)
    {
        drawText("Enter a last name and press Search or Enter.",
            cx, cy, 14, COL_TEXT_MUTED, FONT_REGULAR);
        return;
    }
    if (app.searchResults.empty())
    {
        drawText("No students found.", cx, cy, 14, COL_TEXT_MUTED, FONT_REGULAR);
        return;
    }

    std::string resLabel = "Found: " + std::to_string(app.searchResults.size())
        + " student(s)";
    drawText(resLabel, cx, cy, 14, COL_TEXT, FONT_BOLD);

    if (app.searchUseBinary && app.lastBinaryIdx >= 0)
        drawText("(Binary: index " + std::to_string(app.lastBinaryIdx) + " in sorted array)",
            cx + 200, cy, 12, COL_ACCENT, FONT_REGULAR);

    cy += 28;

    DrawRectangleRounded({ cx, cy, cw, 32 }, 0.15f, 6, COL_SURFACE2);
    drawText("Student", cx + 10, cy + 9, 12, COL_TEXT_LIGHT, FONT_BOLD);
    drawText("Class", cx + 240, cy + 9, 12, COL_TEXT_LIGHT, FONT_BOLD);
    drawText("Average", cx + 340, cy + 9, 12, COL_TEXT_LIGHT, FONT_BOLD);
    drawText("ID", cx + 500, cy + 9, 12, COL_TEXT_LIGHT, FONT_BOLD);
    cy += 32;

    for (int i = 0; i < (int)app.searchResults.size() && cy < WINDOW_HEIGHT - 16; ++i)
    {
        Student* s = app.searchResults[i];
        double   avg = logic_studentAverage(*s);
        bool  binMatch = app.searchUseBinary && (i == app.lastBinaryIdx);
        Color rowBg = binMatch ? Color{ 209, 250, 229, 255 }
        : ((i % 2 == 0) ? COL_SURFACE : COL_BG);

        DrawRectangle((int)cx, (int)cy, (int)cw, ROW_HEIGHT - 2, rowBg);

        std::string name = s->firstName + " " + s->lastName;
        drawText(name, cx + 10, cy + 12, 13, COL_TEXT, FONT_REGULAR);
        drawText(s->classLabel, cx + 240, cy + 12, 13, COL_TEXT_MUTED, FONT_REGULAR);
        pres_gradeBadge(cx + 340, cy + 8, avg, 12);
        drawText(std::to_string(s->id), cx + 500, cy + 12, 12,
            COL_TEXT_MUTED, FONT_REGULAR);
        if (binMatch)
            drawText("<-- match", cx + 540, cy + 12, 12, COL_ACCENT, FONT_REGULAR);

        cy += ROW_HEIGHT - 2;
    }
}

// ═══════════════════════════════════════════════════════════════
//  Sort Demo Screen
// ═══════════════════════════════════════════════════════════════

void pres_drawSortDemoScreen(AppState& app)
{
    float cx = SIDEBAR_WIDTH + 16.0f;
    float cy = TOPBAR_HEIGHT + 16.0f;
    float cw = WINDOW_WIDTH - SIDEBAR_WIDTH - 32.0f;

    drawText("Sort Demo", cx, cy, 20, COL_TEXT, FONT_BOLD);
    cy += 36;

    std::vector<std::string> algos = { "Bubble Sort", "Quick Sort" };
    std::vector<std::string> fields = { "Last Name", "Average", "Class", "ID" };
    std::vector<std::string> orders = { "Ascending", "Descending" };

    int ai = app.demoUseBubble ? 0 : 1;
    int fi = static_cast<int>(app.demoSortField);
    int oi = static_cast<int>(app.demoSortOrder);

    pres_cycleSelector({ cx,       cy, 160, 36 }, "Algo", algos, ai);
    pres_cycleSelector({ cx + 168, cy, 160, 36 }, "Field", fields, fi);
    pres_cycleSelector({ cx + 336, cy, 160, 36 }, "Order", orders, oi);

    app.demoUseBubble = (ai == 0);
    app.demoSortField = static_cast<SortField>(fi);
    app.demoSortOrder = static_cast<SortOrder>(oi);

    if (pres_button({ cx + 514, cy, 110, 36 }, "Sort!", COL_PRIMARY, WHITE, 14))
    {
        app.demoList = data_getAllActive(app.store);
        if (app.demoUseBubble)
            app.demoSwapCount = logic_bubbleSort(app.demoList,
                app.demoSortField,
                app.demoSortOrder);
        else
        {
            logic_quickSort(app.demoList, app.demoSortField, app.demoSortOrder);
            app.demoSwapCount = -1;
        }
    }

    cy += 50;

    if (app.demoUseBubble)
    {
        pres_drawCard({ cx, cy, cw, 52 }, { 255, 251, 235, 255 },
            { 251, 191, 36, 255 }, 0.1f);
        drawText("Bubble Sort: Compares adjacent elements and swaps if out of order.",
            cx + 12, cy + 8, 13, { 146, 64, 14, 255 }, FONT_REGULAR);
        drawText("Complexity: O(n^2) worst case, O(n) on already sorted array (early-exit).",
            cx + 12, cy + 28, 13, { 146, 64, 14, 255 }, FONT_REGULAR);
    }
    else
    {
        pres_drawCard({ cx, cy, cw, 52 }, { 240, 253, 250, 255 },
            { 52, 211, 153, 255 }, 0.1f);
        drawText("Quick Sort: Picks a pivot and partitions the array into two sub-arrays.",
            cx + 12, cy + 8, 13, { 6, 78, 59, 255 }, FONT_REGULAR);
        drawText("Complexity: O(n log n) average, O(n^2) worst case.",
            cx + 12, cy + 28, 13, { 6, 78, 59, 255 }, FONT_REGULAR);
    }

    cy += 64;

    if (!app.demoList.empty())
    {
        std::string info;
        if (app.demoSwapCount >= 0)
            info = "Bubble Sort: " + std::to_string(app.demoSwapCount)
            + " swaps for " + std::to_string(app.demoList.size()) + " elements.";
        else
            info = "Quick Sort completed for "
            + std::to_string(app.demoList.size()) + " elements.";

        drawText(info, cx, cy, 13, COL_ACCENT, FONT_BOLD);
        cy += 28;

        DrawRectangleRounded({ cx, cy, cw, 32 }, 0.15f, 6, COL_SURFACE2);
        drawText("Rank", cx + 8, cy + 9, 12, COL_TEXT_LIGHT, FONT_BOLD);
        drawText("Student", cx + 60, cy + 9, 12, COL_TEXT_LIGHT, FONT_BOLD);
        drawText("Class", cx + 280, cy + 9, 12, COL_TEXT_LIGHT, FONT_BOLD);
        drawText("Average", cx + 380, cy + 9, 12, COL_TEXT_LIGHT, FONT_BOLD);
        cy += 32;

        for (int i = 0; i < (int)app.demoList.size() && cy < WINDOW_HEIGHT - 10; ++i)
        {
            Student* s = app.demoList[i];
            double   avg = logic_studentAverage(*s);
            Color    rowBg = (i % 2 == 0) ? COL_SURFACE : COL_BG;
            DrawRectangle((int)cx, (int)cy, (int)cw, 34, rowBg);

            drawText(std::to_string(i + 1), cx + 8, cy + 10, 12, COL_TEXT_MUTED, FONT_REGULAR);
            std::string name = s->firstName + " " + s->lastName;
            drawText(name, cx + 60, cy + 10, 13, COL_TEXT, FONT_REGULAR);
            drawText(s->classLabel, cx + 280, cy + 10, 13, COL_TEXT_MUTED, FONT_REGULAR);
            pres_gradeBadge(cx + 380, cy + 6, avg, 12);
            cy += 34;
        }
    }
    else
    {
        pres_drawCard({ cx, cy, cw, 60 }, COL_SURFACE, COL_BORDER, 0.1f);
        drawText("Press 'Sort!' to see the result.",
            cx + 20, cy + 20, 14, COL_TEXT_MUTED, FONT_REGULAR);
    }
}

// ═══════════════════════════════════════════════════════════════
//  Stats Screen
// ═══════════════════════════════════════════════════════════════

void pres_drawStatsScreen(AppState& app)
{
    float cx = SIDEBAR_WIDTH + 16.0f;
    float cy = TOPBAR_HEIGHT + 16.0f;
    float cw = WINDOW_WIDTH - SIDEBAR_WIDTH - 32.0f;

    drawText("Statistics", cx, cy, 20, COL_TEXT, FONT_BOLD);
    cy += 36;

    std::vector<std::string> classOpts;
    classOpts.push_back("All");
    for (const std::string& cl : data_getAllClassLabels(app.store))
        classOpts.push_back(cl);

    int classIdx = 0;
    for (int i = 0; i < (int)classOpts.size(); ++i)
        if (classOpts[i] == app.statsClassFilter ||
            (app.statsClassFilter.empty() && i == 0))
        {
            classIdx = i; break;
        }

    if (pres_cycleSelector({ cx, cy, 200, 36 }, "Class", classOpts, classIdx))
    {
        app.statsClassFilter = (classIdx == 0) ? "" : classOpts[classIdx];
        app.statsNeedsRefresh = true;
    }

    if (app.statsNeedsRefresh)
    {
        app.currentStats = app.statsClassFilter.empty()
            ? logic_schoolStats(app.store)
            : logic_classStats(app.store, app.statsClassFilter);
        app.statsNeedsRefresh = false;
    }

    cy += 50;

    const GroupStats& gs = app.currentStats;
    float cardW = (cw - 48) / 4.0f;
    float cardH = 84.0f;

    pres_statCard({ cx,                  cy, cardW, cardH },
        std::to_string(gs.totalStudents), "Students", COL_PRIMARY);
    pres_statCard({ cx + cardW + 16,     cy, cardW, cardH },
        pres_fmt2(gs.overallAverage), "Avg. Grade", COL_ACCENT);
    pres_statCard({ cx + 2 * (cardW + 16), cy, cardW, cardH },
        std::to_string(gs.excellentCount), "Excellent", { 234, 179, 8, 255 });
    pres_statCard({ cx + 3 * (cardW + 16), cy, cardW, cardH },
        std::to_string(gs.totalFailing), "Failing", COL_GRADE[0]);

    cy += cardH + 24;

    pres_drawCard({ cx, cy, cw, 48 }, { 238, 242, 255, 255 },
        { 129, 140, 248, 255 }, 0.1f);
    {
        std::vector<Student*> allS = app.statsClassFilter.empty()
            ? data_getAllActive(app.store)
            : data_findByClass(app.store, app.statsClassFilter);
        int recFailing = logic_recursiveCountFailing(allS, 0);
        std::string recStr = "Recursive count: "
            + std::to_string(recFailing)
            + " student(s) with at least one failing grade.";
        drawText(recStr, cx + 12, cy + 16, 13, { 67, 56, 202, 255 }, FONT_REGULAR);
    }
    cy += 60;

    drawText("Detailed Statistics by Subject", cx, cy, 15, COL_TEXT, FONT_BOLD);
    cy += 26;

    DrawRectangleRounded({ cx, cy, cw, 30 }, 0.15f, 6, COL_SURFACE2);
    float col[] = { cx + 10, cx + 170, cx + 280, cx + 380, cx + 460, cx + 560, cx + 660 };
    const char* hdrs[] = { "Subject", "Average", "Highest", "Lowest",
                            "Count", "Failing", "Chart" };
    for (int i = 0; i < 7; ++i)
        drawText(hdrs[i], col[i], cy + 8, 12, COL_TEXT_LIGHT, FONT_BOLD);
    cy += 30;

    for (const SubjectStats& ss : gs.bySubject)
    {
        DrawRectangle((int)cx, (int)cy, (int)cw, 30, COL_SURFACE);
        drawText(data_subjectName(ss.subject), col[0], cy + 8, 13, COL_TEXT, FONT_REGULAR);
        pres_gradeBadge(col[1], cy + 5, ss.average, 12);
        drawText(pres_fmt2(ss.highest), col[2], cy + 8, 12, COL_GRADE[4], FONT_REGULAR);
        drawText(pres_fmt2(ss.lowest), col[3], cy + 8, 12, COL_GRADE[0], FONT_REGULAR);
        drawText(std::to_string(ss.count), col[4], cy + 8, 12, COL_TEXT_MUTED, FONT_REGULAR);
        drawText(std::to_string(ss.failing), col[5], cy + 8, 12,
            ss.failing > 0 ? COL_GRADE[0] : COL_ACCENT, FONT_REGULAR);

        float fill = (float)((ss.average - 2.0) / 4.0);
        int   ci = logic_gradeColorIndex(ss.average);
        Color fc = (ci < 0) ? COL_BORDER : COL_GRADE[ci];
        pres_progressBar({ col[6], cy + 8, 100, 14 }, fill, fc, COL_BORDER);

        cy += 30;
        if (cy > WINDOW_HEIGHT - 32) break;
    }

    cy += 10;
    pres_drawGradeLegend(cx, cy);
}

// ═══════════════════════════════════════════════════════════════
//  About Screen
// ═══════════════════════════════════════════════════════════════

void pres_drawAboutScreen(AppState& app)
{
    float cx = SIDEBAR_WIDTH + 16.0f;
    float cy = TOPBAR_HEIGHT + 16.0f;
    float cw = WINDOW_WIDTH - SIDEBAR_WIDTH - 32.0f;

    drawText("About", cx, cy, 22, COL_TEXT, FONT_BOLD);
    cy += 36;

    struct Section { const char* title; const char* body; };
    Section sections[] = {
        { "Goal",
          "GradeBook is a C++ application for managing student grades,\n"
          "developed as the second project for 9th grade.\n"
          "It implements a three-tier architecture: Data / Logic / Presentation." },
        { "Algorithms",
          "Sorting: Bubble Sort (O(n^2)), Quick Sort (O(n log n))\n"
          "Searching: Linear (O(n)), Binary (O(log n))\n"
          "Recursion: Average grade, Fibonacci, Factorial, Failing counter" },
        { "GUI Framework",
          "Raylib (https://www.raylib.com) - lightweight C graphics library.\n"
          "All controls are implemented manually in presentation.cpp." },
        { "Architecture",
          "data.h / data.cpp    -- structures and file I/O\n"
          "logic.h / logic.cpp  -- algorithms, validation, statistics\n"
          "presentation.h / .cpp -- GUI, input handling, visualization\n"
          "main.cpp             -- entry point; initializes window and loop" },
        { "Team",
          "Scrum Master: [Your Name]\n"
          "Back-End Developer 1: [Your Name]\n"
          "Back-End Developer 2: [Your Name]\n"
          "Front-End Developer: [Your Name]" },
    };

    for (const Section& sec : sections)
    {
        pres_drawCard({ cx, cy, cw, 110 }, COL_SURFACE, COL_BORDER, 0.1f);
        DrawRectangleRounded({ cx, cy, cw, 28 }, 0.1f, 6, COL_PRIMARY);
        drawText(sec.title, cx + 12, cy + 7, 14, WHITE, FONT_BOLD);

        std::string body(sec.body);
        float by = cy + 36;
        std::string line;
        for (char c : body)
        {
            if (c == '\n')
            {
                drawText(line, cx + 12, by, 13, COL_TEXT, FONT_REGULAR);
                by += 18;
                line.clear();
            }
            else line += c;
        }
        if (!line.empty())
            drawText(line, cx + 12, by, 13, COL_TEXT, FONT_REGULAR);

        cy += 118;
    }

    cy += 8;
    drawText("Fibonacci(10) = " + std::to_string(logic_fibonacci(10))
        + "   |   Factorial(7) = " + std::to_string(logic_factorial(7)),
        cx, cy, 13, COL_TEXT_MUTED, FONT_REGULAR);
}

// ═══════════════════════════════════════════════════════════════
//  Modals
// ═══════════════════════════════════════════════════════════════

static void drawOverlay()
{
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, { 15, 23, 42, 160 });
}

static Rectangle modalRect(float w, float h)
{
    return { (WINDOW_WIDTH - w) / 2.0f, (WINDOW_HEIGHT - h) / 2.0f, w, h };
}

void pres_drawModalAddStudent(AppState& app)
{
    drawOverlay();
    Rectangle r = modalRect(480, 420);
    pres_drawCard(r, COL_SURFACE, COL_BORDER, 0.1f);

    float px = r.x + 20, py = r.y + 16;

    drawText("New Student", px, py, 18, COL_TEXT, FONT_BOLD);
    py += 36;

    drawText("First Name:", px, py, 13, COL_TEXT_MUTED, FONT_REGULAR);
    pres_textInput({ px, py + 18, 420, 36 }, app.formFirst, INPUT_BUF,
        "First name...", app.focusedInput, 10, 14);
    py += 64;

    drawText("Last Name:", px, py, 13, COL_TEXT_MUTED, FONT_REGULAR);
    pres_textInput({ px, py + 18, 420, 36 }, app.formLast, INPUT_BUF,
        "Last name...", app.focusedInput, 11, 14);
    py += 64;

    drawText("Class:", px, py, 13, COL_TEXT_MUTED, FONT_REGULAR);
    pres_textInput({ px, py + 18, 200, 36 }, app.formClass, INPUT_BUF,
        "e.g. 9A", app.focusedInput, 12, 14);

    std::vector<std::string> genders = { "Male", "Female", "Other" };
    pres_cycleSelector({ px + 220, py + 18, 200, 36 }, "Gender", genders, app.formGender);
    py += 64;

    std::vector<std::string> years = { "8th grade", "9th grade", "10th grade", "11th grade" };
    int yr = app.formYearGroup - 1;
    pres_cycleSelector({ px, py, 420, 36 }, "Year", years, yr);
    app.formYearGroup = yr + 1;
    py += 50;

    if (!app.formError.empty())
        drawText(app.formError, px, py - 10, 12, COL_GRADE[0], FONT_REGULAR);

    if (pres_button({ px, py + 8, 196, 38 }, "Add Student", COL_PRIMARY, WHITE, 15))
    {
        Student s;
        s.firstName = app.formFirst;
        s.lastName = app.formLast;
        s.classLabel = app.formClass;
        s.gender = static_cast<Gender>(app.formGender);
        s.yearGroup = static_cast<Grade>(app.formYearGroup);
        s.isActive = true;
        int id;
        LogicResult lr = logic_addStudent(app.store, s, id);
        if (lr == LogicResult::OK)
        {
            app.displayedStudents = data_getAllActive(app.store);
            logic_quickSort(app.displayedStudents, app.sortField, app.sortOrder);
            app.statsNeedsRefresh = true;
            app.activeModal = Modal::NONE;
            pres_notify(app, "Student added successfully.");
        }
        else
            app.formError = logic_resultMessage(lr);
    }

    if (pres_buttonGhost({ r.x + 240, py + 8, 196, 38 }, "Cancel", 15))
        app.activeModal = Modal::NONE;

    if (IsKeyPressed(KEY_ESCAPE)) app.activeModal = Modal::NONE;
}

void pres_drawModalEditStudent(AppState& app)
{
    drawOverlay();
    Rectangle r = modalRect(480, 380);
    pres_drawCard(r, COL_SURFACE, COL_BORDER, 0.1f);

    float px = r.x + 20, py = r.y + 16;

    drawText("Edit Student", px, py, 18, COL_TEXT, FONT_BOLD);
    py += 36;

    drawText("First Name:", px, py, 13, COL_TEXT_MUTED, FONT_REGULAR);
    pres_textInput({ px, py + 18, 420, 36 }, app.formFirst, INPUT_BUF,
        "First name...", app.focusedInput, 20, 14);
    py += 64;

    drawText("Last Name:", px, py, 13, COL_TEXT_MUTED, FONT_REGULAR);
    pres_textInput({ px, py + 18, 420, 36 }, app.formLast, INPUT_BUF,
        "Last name...", app.focusedInput, 21, 14);
    py += 64;

    drawText("Class:", px, py, 13, COL_TEXT_MUTED, FONT_REGULAR);
    pres_textInput({ px, py + 18, 200, 36 }, app.formClass, INPUT_BUF,
        "e.g. 9A", app.focusedInput, 22, 14);

    std::vector<std::string> genders = { "Male", "Female", "Other" };
    pres_cycleSelector({ px + 220, py + 18, 200, 36 }, "Gender", genders, app.formGender);
    py += 64;

    if (!app.formError.empty())
        drawText(app.formError, px, py - 10, 12, COL_GRADE[0], FONT_REGULAR);

    if (pres_button({ px, py + 8, 196, 38 }, "Save", COL_ACCENT, WHITE, 15))
    {
        Student* orig = data_findById(app.store, app.editStudentId);
        if (orig)
        {
            Student updated = *orig;
            updated.firstName = app.formFirst;
            updated.lastName = app.formLast;
            updated.classLabel = app.formClass;
            updated.gender = static_cast<Gender>(app.formGender);
            LogicResult lr = logic_updateStudent(app.store, updated);
            if (lr == LogicResult::OK)
            {
                app.displayedStudents = data_getAllActive(app.store);
                logic_quickSort(app.displayedStudents, app.sortField, app.sortOrder);
                app.statsNeedsRefresh = true;
                app.activeModal = Modal::NONE;
                pres_notify(app, "Student updated successfully.");
            }
            else
                app.formError = logic_resultMessage(lr);
        }
    }

    if (pres_buttonGhost({ r.x + 240, py + 8, 196, 38 }, "Cancel", 15))
        app.activeModal = Modal::NONE;

    if (IsKeyPressed(KEY_ESCAPE)) app.activeModal = Modal::NONE;
}

void pres_drawModalDeleteConfirm(AppState& app)
{
    drawOverlay();
    Rectangle r = modalRect(400, 180);
    pres_drawCard(r, COL_SURFACE, COL_GRADE[0], 0.1f);

    float px = r.x + 20, py = r.y + 20;

    drawText("Delete Student", px, py, 18, COL_GRADE[0], FONT_BOLD);
    py += 36;

    Student* s = data_findById(app.store, app.deleteTargetId);
    std::string name = s ? s->firstName + " " + s->lastName : "?";
    drawText("Are you sure you want to delete: " + name + "?",
        px, py, 13, COL_TEXT, FONT_REGULAR);
    py += 40;

    if (pres_button({ px, py, 160, 38 }, "Yes, Delete!", COL_GRADE[0], WHITE, 14))
    {
        logic_deleteStudent(app.store, app.deleteTargetId);
        app.displayedStudents = data_getAllActive(app.store);
        logic_quickSort(app.displayedStudents, app.sortField, app.sortOrder);
        app.statsNeedsRefresh = true;
        app.activeModal = Modal::NONE;
        pres_notify(app, "Student deleted.", { 239, 68, 68, 255 });
    }

    if (pres_buttonGhost({ r.x + 196, py, 160, 38 }, "Cancel", 14))
        app.activeModal = Modal::NONE;

    if (IsKeyPressed(KEY_ESCAPE)) app.activeModal = Modal::NONE;
}

void pres_drawModalAddGrade(AppState& app)
{
    drawOverlay();
    Rectangle r = modalRect(420, 260);
    pres_drawCard(r, COL_SURFACE, COL_BORDER, 0.1f);

    float px = r.x + 20, py = r.y + 16;

    drawText("Grade - " + data_subjectName(app.gradeSubject),
        px, py, 17, COL_TEXT, FONT_BOLD);
    py += 40;

    drawText("Grade value (2.00 - 6.00):", px, py, 13, COL_TEXT_MUTED, FONT_REGULAR);
    pres_textInput({ px, py + 18, 380, 38 }, app.gradeValueBuf, 16,
        "e.g. 5.50", app.focusedInput, 30, 15);
    py += 66;

    drawText("Note (optional):", px, py, 13, COL_TEXT_MUTED, FONT_REGULAR);
    pres_textInput({ px, py + 18, 380, 38 }, app.gradeNoteBuf, INPUT_BUF,
        "Note...", app.focusedInput, 31, 14);
    py += 66;

    if (!app.gradeError.empty())
        drawText(app.gradeError, px, py - 8, 12, COL_GRADE[0], FONT_REGULAR);

    if (pres_button({ px, py, 175, 38 }, "Save Grade", COL_PRIMARY, WHITE, 13))
    {
        try
        {
            double val = std::stod(app.gradeValueBuf);
            LogicResult lr = logic_setGrade(app.store, app.gradeStudentId,
                app.gradeSubject, val, app.gradeNoteBuf);
            if (lr == LogicResult::OK)
            {
                app.statsNeedsRefresh = true;
                app.activeModal = Modal::NONE;
                pres_notify(app, "Grade saved.");
            }
            else
                app.gradeError = logic_resultMessage(lr);
        }
        catch (...) {
            app.gradeError = "Invalid value. Enter a number between 2 and 6.";
        }
    }

    if (pres_buttonGhost({ r.x + 215, py, 165, 38 }, "Cancel", 13))
        app.activeModal = Modal::NONE;

    if (IsKeyPressed(KEY_ESCAPE)) app.activeModal = Modal::NONE;
}

void pres_drawModalStudentDetail(AppState& app)
{
    drawOverlay();
    Student* s = data_findById(app.store, app.selectedStudentId);
    if (!s) { app.activeModal = Modal::NONE; return; }

    Rectangle r = modalRect(600, 520);
    pres_drawCard(r, COL_SURFACE, COL_BORDER, 0.1f);

    DrawRectangleRounded({ r.x, r.y, r.width, 52 }, 0.1f, 6, COL_PRIMARY);
    drawText(s->firstName + " " + s->lastName,
        r.x + 16, r.y + 12, 20, WHITE, FONT_BOLD);
    drawText(s->classLabel, r.x + 16, r.y + 34, 12,
        { 147, 197, 253, 255 }, FONT_REGULAR);

    float px = r.x + 16, py = r.y + 66;

    drawText("Gender: " + data_genderName(s->gender),
        px, py, 13, COL_TEXT_MUTED, FONT_REGULAR);
    drawText("Year: " + data_gradeName(s->yearGroup),
        px + 200, py, 13, COL_TEXT_MUTED, FONT_REGULAR);

    double avg = logic_studentAverage(*s);
    drawText("Average: " + pres_fmtGrade(avg),
        px + 400, py, 13, COL_TEXT, FONT_BOLD);

    py += 30;
    pres_separator(r.x + 10, py, r.width - 20, COL_BORDER);
    py += 14;

    double recAvg = logic_recursiveAverage(s->grades, 0, 0.0);
    drawText("Recursive average: " + pres_fmt2(recAvg),
        px, py, 12, { 99, 102, 241, 255 }, FONT_REGULAR);
    py += 26;

    auto allSubj = data_allSubjects();
    int  cols = 2;
    float cellW = (r.width - 40) / cols;
    float cellH = 44.0f;

    for (int i = 0; i < (int)allSubj.size(); ++i)
    {
        Subject subj = allSubj[i];
        double  g = data_getGrade(*s, subj);
        int     col = i % cols;
        int     row = i / cols;

        float rx = px + col * (cellW + 8);
        float ry = py + row * (cellH + 6);

        DrawRectangle((int)rx, (int)ry, (int)cellW - 4, (int)cellH,
            (i % 2 == 0) ? COL_SURFACE : COL_BG);
        drawText(data_subjectName(subj), rx + 8, ry + 8, 12, COL_TEXT, FONT_REGULAR);
        pres_gradeBadge(rx + cellW - 76, ry + 8, g, 12);
    }

    py += (((int)allSubj.size() + 1) / 2) * (cellH + 6) + 10;

    if (pres_button({ r.x + r.width - 120, r.y + r.height - 54, 100, 38 },
        "Close", COL_PRIMARY, WHITE, 14)
        || IsKeyPressed(KEY_ESCAPE))
        app.activeModal = Modal::NONE;
}

// ═══════════════════════════════════════════════════════════════
//  Main update loop
// ═══════════════════════════════════════════════════════════════

void pres_update(AppState& app)
{
    BeginDrawing();
    ClearBackground(COL_BG);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && app.activeModal == Modal::NONE)
        app.focusedInput = 0;

    // Show login screen if not authenticated
    if (app.currentScreen == Screen::LOGIN)
    {
        pres_drawLoginScreen(app);
        EndDrawing();
        return;
    }

    pres_drawSidebar(app);
    pres_drawTopBar(app);

    auto drawScreen = [&]() {
        switch (app.currentScreen)
        {
        case Screen::DASHBOARD: pres_drawDashboard(app);      break;
        case Screen::STUDENTS:  pres_drawStudentsScreen(app);  break;
        case Screen::GRADES:    pres_drawGradesScreen(app);    break;
        case Screen::SEARCH:    pres_drawSearchScreen(app);    break;
        case Screen::SORT_DEMO: pres_drawSortDemoScreen(app);  break;
        case Screen::STATS:     pres_drawStatsScreen(app);     break;
        case Screen::ABOUT:     pres_drawAboutScreen(app);     break;
        }
        };

    drawScreen();

    if (app.activeModal != Modal::NONE)
    {
        switch (app.activeModal)
        {
        case Modal::ADD_STUDENT:    pres_drawModalAddStudent(app);    break;
        case Modal::EDIT_STUDENT:   pres_drawModalEditStudent(app);   break;
        case Modal::DELETE_CONFIRM: pres_drawModalDeleteConfirm(app); break;
        case Modal::ADD_GRADE:      pres_drawModalAddGrade(app);      break;
        case Modal::STUDENT_DETAIL: pres_drawModalStudentDetail(app); break;
        default: break;
        }
    }

    EndDrawing();
}
