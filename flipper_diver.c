#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>

#define TAG "FlipperDiver"

#define INPUT_QUEUE_SIZE 8

typedef struct {
    FuriMutex* mutex; /* protects last_key, read by the GUI thread */
    InputKey last_key;
    bool has_key;
} FlipperDiverState;

static const char* flipper_diver_arrow(InputKey key) {
    switch(key) {
    case InputKeyUp:
        return "Up";
    case InputKeyDown:
        return "Down";
    case InputKeyLeft:
        return "Left";
    case InputKeyRight:
        return "Right";
    default:
        return input_get_key_name(key);
    }
}

static void flipper_diver_draw_callback(Canvas* canvas, void* ctx) {
    FlipperDiverState* state = ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Flipper Diver");

    canvas_set_font(canvas, FontSecondary);
    furi_check(furi_mutex_acquire(state->mutex, FuriWaitForever) == FuriStatusOk);
    const char* label = state->has_key ? flipper_diver_arrow(state->last_key) : "Press an arrow";
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, label);
    furi_mutex_release(state->mutex);

    canvas_draw_str_aligned(canvas, 64, 62, AlignCenter, AlignBottom, "Back to exit");
}

static void flipper_diver_input_callback(InputEvent* event, void* ctx) {
    FuriMessageQueue* queue = ctx;
    /* Called from the input thread: never block it, drop the event if full. */
    furi_message_queue_put(queue, event, 0);
}

int32_t flipper_diver_app(void* p) {
    UNUSED(p);

    FlipperDiverState state = {
        .mutex = furi_mutex_alloc(FuriMutexTypeNormal),
        .has_key = false,
    };
    FuriMessageQueue* queue = furi_message_queue_alloc(INPUT_QUEUE_SIZE, sizeof(InputEvent));

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, flipper_diver_draw_callback, &state);
    view_port_input_callback_set(view_port, flipper_diver_input_callback, queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    FURI_LOG_I(TAG, "started");

    InputEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(queue, &event, FuriWaitForever) != FuriStatusOk) {
            continue;
        }
        if(event.type != InputTypeShort) {
            continue;
        }
        if(event.key == InputKeyBack) {
            running = false;
            continue;
        }

        furi_check(furi_mutex_acquire(state.mutex, FuriWaitForever) == FuriStatusOk);
        state.last_key = event.key;
        state.has_key = true;
        furi_mutex_release(state.mutex);
        view_port_update(view_port);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(queue);
    furi_mutex_free(state.mutex);

    return 0;
}
