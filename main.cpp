
#include "presentation.h"
#include "raylib.h"

int main()
{
    // ── Window initialisation ────────────────────────────────
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
        "GradeBook – Система за управление на оценки");
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);   // Disable default ESC-to-exit behaviour

    // ── Application state initialisation ─────────────────────
    AppState app;
    pres_init(app);

    // ── Main loop ─────────────────────────────────────────────
    while (!WindowShouldClose())
    {
        // Allow ALT+F4 / window X button to quit
        pres_update(app);
    }

    // ── Cleanup ───────────────────────────────────────────────
    pres_shutdown(app);
    CloseWindow();

    return 0;
}
