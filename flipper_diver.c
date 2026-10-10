#include <furi.h>
#include <furi_hal_random.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>

#include "best_score.h"
#include "game.h"
#include "view.h"

#define TAG "FlipperDiver"

#define EVENT_QUEUE_SIZE 16
/* 30 Hz, above the 25 Hz of REQ-001-24. */
#define TICK_PERIOD_MS   33
/* No redraw for this long while a round runs: the screen is taken. */
#define SCREEN_LOST_MS   250

/* Set to 1 to log, at each round end and game over, the longest gap between
 * two frames during the round, the longest delay between a key press and
 * the frame showing it, and the free heap (REQ-001-24, 25). */
#define FLIPPER_DIVER_METRICS 0

typedef enum {
    AppEventTypeInput,
    AppEventTypeTick,
} AppEventType;

typedef struct {
    AppEventType type;
    InputEvent input;
    uint32_t tick; /* furi_get_tick() when the input arrived */
} AppEvent;

typedef struct {
    uint32_t input_tick; /* key press waiting for its frame, 0 if none */
    uint32_t max_frame_gap;
    uint32_t max_input_latency;
} AppMetrics;

typedef struct {
    FuriMessageQueue* queue;
    FuriTimer* timer;
    ViewPort* view_port;
    Gui* gui;
    NotificationApp* notifications;

    FuriMutex* mutex; /* protects game, read by the GUI thread */
    Game game;

    uint32_t last_tick; /* main thread only */
    uint32_t last_draw_tick; /* protected by mutex, written by the GUI thread */
    AppMetrics metrics; /* protected by mutex */
} App;

static uint32_t ticks_to_ms(uint32_t ticks) {
    return (uint32_t)((uint64_t)ticks * 1000u / furi_kernel_get_tick_frequency());
}

/* Called with the lock held, from the draw callback. */
static void app_metrics_on_draw(App* app, uint32_t now) {
    if(!FLIPPER_DIVER_METRICS) {
        return;
    }
    AppMetrics* metrics = &app->metrics;
    if(app->game.phase == GamePhasePlaying) {
        uint32_t gap = ticks_to_ms(now - app->last_draw_tick);
        if(gap > metrics->max_frame_gap) {
            metrics->max_frame_gap = gap;
        }
    }
    if(metrics->input_tick) {
        uint32_t latency = ticks_to_ms(now - metrics->input_tick);
        if(latency > metrics->max_input_latency) {
            metrics->max_input_latency = latency;
        }
        metrics->input_tick = 0;
    }
}

/* Called with the lock held, from the main loop. */
static void app_metrics_on_events(App* app, const AppEvent* event, GameEvents events) {
    if(!FLIPPER_DIVER_METRICS) {
        return;
    }
    AppMetrics* metrics = &app->metrics;
    if(event->type == AppEventTypeInput && (events & (GameEventArrowOk | GameEventError))) {
        metrics->input_tick = event->tick ? event->tick : 1;
    }
    if(events & (GameEventRoundDone | GameEventGameOver)) {
        FURI_LOG_I(
            TAG,
            "metrics: round %lu, max frame gap %lu ms, max input latency %lu ms, free heap %zu",
            (unsigned long)app->game.round,
            (unsigned long)metrics->max_frame_gap,
            (unsigned long)metrics->max_input_latency,
            memmgr_get_free_heap());
        metrics->max_frame_gap = 0;
        metrics->max_input_latency = 0;
    }
}

/* Wrong arrow: short vibration and red LED (REQ-001-7). Like every sequence
 * here, it goes through the notification service, which applies the
 * Flipper's vibration and LED settings (REQ-001-22). */
static const NotificationSequence sequence_wrong_arrow = {
    &message_vibro_on,
    &message_red_255,
    &message_delay_100,
    &message_vibro_off,
    &message_red_0,
    NULL,
};

static void app_notify(App* app, GameEvents events) {
    if(events & GameEventError) {
        notification_message(app->notifications, &sequence_wrong_arrow);
    }
    if(events & GameEventGameOver) {
        notification_message(app->notifications, &sequence_error);
    } else if(events & GameEventRoundDone) {
        notification_message(app->notifications, &sequence_success);
    } else if(events & GameEventStratagemDone) {
        notification_message(app->notifications, &sequence_blink_green_10);
    }
}

static void app_draw_callback(Canvas* canvas, void* ctx) {
    App* app = ctx;
    furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);
    view_draw(canvas, &app->game);
    uint32_t now = furi_get_tick();
    app_metrics_on_draw(app, now);
    app->last_draw_tick = now;
    furi_mutex_release(app->mutex);
}

static void app_input_callback(InputEvent* input, void* ctx) {
    App* app = ctx;
    AppEvent event = {.type = AppEventTypeInput, .input = *input, .tick = furi_get_tick()};
    /* Input thread: never block it, drop the event if the queue is full. */
    furi_message_queue_put(app->queue, &event, 0);
}

static void app_timer_callback(void* ctx) {
    App* app = ctx;
    AppEvent event = {.type = AppEventTypeTick};
    /* A dropped tick is harmless: the next one catches up on elapsed time. */
    furi_message_queue_put(app->queue, &event, 0);
}

/* Maps an input event to a game key: arrows count on press (REQ-001-5),
 * Ok and Back on short press. Returns false for events the game ignores. */
static bool app_game_key(const InputEvent* input, GameKey* key) {
    switch(input->key) {
    case InputKeyUp:
        *key = GameKeyUp;
        return input->type == InputTypePress;
    case InputKeyDown:
        *key = GameKeyDown;
        return input->type == InputTypePress;
    case InputKeyLeft:
        *key = GameKeyLeft;
        return input->type == InputTypePress;
    case InputKeyRight:
        *key = GameKeyRight;
        return input->type == InputTypePress;
    case InputKeyOk:
        *key = GameKeyOk;
        return input->type == InputTypeShort;
    case InputKeyBack:
        *key = GameKeyBack;
        return input->type == InputTypeShort;
    default:
        return false;
    }
}

/* Time elapsed since the previous call, in ms. */
static uint32_t app_elapsed_ms(App* app) {
    uint32_t now = furi_get_tick();
    uint32_t ticks = now - app->last_tick;
    app->last_tick = now;
    return ticks_to_ms(ticks);
}

static App* app_alloc(void) {
    App* app = malloc(sizeof(App));
    app->queue = furi_message_queue_alloc(EVENT_QUEUE_SIZE, sizeof(AppEvent));
    app->timer = furi_timer_alloc(app_timer_callback, FuriTimerTypePeriodic, app);
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    uint32_t best_score;
    uint32_t best_round;
    best_score_load(&best_score, &best_round);
    game_init(&app->game, furi_hal_random_get(), best_score, best_round);

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, app_draw_callback, app);
    view_port_input_callback_set(app->view_port, app_input_callback, app);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->last_tick = furi_get_tick();
    app->last_draw_tick = app->last_tick;
    furi_timer_start(app->timer, furi_ms_to_ticks(TICK_PERIOD_MS));
    return app;
}

static void app_free(App* app) {
    furi_timer_stop(app->timer);
    furi_record_close(RECORD_NOTIFICATION);
    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(app->view_port);
    furi_mutex_free(app->mutex);
    furi_timer_free(app->timer);
    furi_message_queue_free(app->queue);
    free(app);
}

int32_t flipper_diver_app(void* p) {
    UNUSED(p);
    App* app = app_alloc();
    FURI_LOG_I(TAG, "started");

    AppEvent event;
    bool running = true;
    while(running) {
        if(furi_message_queue_get(app->queue, &event, FuriWaitForever) != FuriStatusOk) {
            continue;
        }

        furi_check(furi_mutex_acquire(app->mutex, FuriWaitForever) == FuriStatusOk);
        /* The timer is brought up to date before any key (design.md). */
        /* The GUI only calls the draw callback of the visible ViewPort: no
         * redraw although we keep asking for one means another ViewPort
         * covers ours, so the round pauses (REQ-001-18, design.md). */
        if(event.type == AppEventTypeTick &&
           furi_get_tick() - app->last_draw_tick > furi_ms_to_ticks(SCREEN_LOST_MS)) {
            game_pause(&app->game);
        }
        GameEvents events = game_advance(&app->game, app_elapsed_ms(app));
        GameKey key;
        if(event.type == AppEventTypeInput && app_game_key(&event.input, &key)) {
            events |= game_key(&app->game, key);
        }
        app_metrics_on_events(app, &event, events);
        furi_mutex_release(app->mutex);

        app_notify(app, events);
        if(events & GameEventNewRecord) {
            /* Only this thread writes the game: no lock needed to read. */
            best_score_save(app->game.best_score, app->game.best_round);
        }
        if(events & GameEventQuit) {
            running = false;
        }
        /* Inputs redraw at once rather than on the next tick (REQ-001-24). */
        view_port_update(app->view_port);
    }

    app_free(app);
    return 0;
}
