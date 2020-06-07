#include "libretro.h"

#include "ww_backgrnd.h"
#include "ww_config.h"
#include "ww_filesys.h"
#include "ww_tile.h"
#include "ww_version.h"

static void dummy_log(enum retro_log_level const level, char const* const fmt, ...) {
    (void)level;
    (void)fmt;
}

static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_environment_t env_cb;
static retro_log_printf_t log_cb = dummy_log;
static retro_audio_sample_batch_t audio_cb;

static uint32_t canvas[WW_SCREEN_WIDTH * WW_SCREEN_HEIGHT * 4];
static ww_backgrnd_t backgrnd;

void retro_get_system_info(struct retro_system_info* const info) {
    info->library_name = WW_PACKAGE;
    info->library_version = WW_VERSION " (" WW_GITHASH ")";
    info->need_fullpath = true;
    info->block_extract = false;
    info->valid_extensions = "wwg|wwl|wwm";
}

void retro_set_environment(retro_environment_t const cb) {
    env_cb = cb;

    bool yes = true;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &yes);
}

unsigned retro_api_version(void) {
    return RETRO_API_VERSION;
}

void retro_init(void) {
    struct retro_log_callback log;

    if (env_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log)) {
        log_cb = log.log;
    }
}

bool retro_load_game(struct retro_game_info const* const info) {
    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;

    if (!env_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
        log_cb(RETRO_LOG_ERROR, WW_PACKAGE " needs XRGB8888\n");
        return false;
    }

    if (ww_backgrnd_init(&backgrnd) != 0) {
        return false;
    }

    if (ww_filesys_init(info->path) != 0) {
error1:
        ww_backgrnd_destroy(&backgrnd);
        return false;
    }

    if (ww_tile_init() != 0) {
error2:
        ww_filesys_destroy();
        goto error1;
    }

    if (ww_tile_load(0, "/black.png") != 0 || ww_tile_load(1, "/test.png") != 0) {
        ww_tile_destroy();
        goto error2;
    }

    return true;
}

size_t retro_get_memory_size(unsigned const id) {
    return 0;
}

void* retro_get_memory_data(unsigned id) {
    return NULL;
}

void retro_set_video_refresh(retro_video_refresh_t cb) {
    video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb) {
    (void)cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
    audio_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb) {
    (void)cb;
}

void retro_set_input_poll(retro_input_poll_t cb) {
    input_poll_cb = cb;
}

void retro_get_system_av_info(struct retro_system_av_info* const info) {
    info->geometry.base_width = WW_SCREEN_WIDTH;
    info->geometry.base_height = WW_SCREEN_HEIGHT;
    info->geometry.max_width = WW_SCREEN_WIDTH * 2;
    info->geometry.max_height = WW_SCREEN_HEIGHT * 2;
    info->geometry.aspect_ratio = 0.0f;
    info->timing.fps = 60;
    info->timing.sample_rate = 44100;
}

void retro_run(void) {
    static int x = 0, y = 0;
    static int dx = 1, dy = 1;
    static unsigned db = 0;

    x += dx;

    if (x < -WW_TILE_SIZE || x > WW_SCREEN_WIDTH) {
        dx = -dx;
        x += dx;
    }

    y += dy;

    if (y < -WW_TILE_SIZE || y > WW_SCREEN_HEIGHT) {
        dy = -dy;
        y += dy;
    }

    db++;

    ww_backgrnd_clear(&backgrnd, 0);
    ww_backgrnd_render(&backgrnd, canvas, WW_SCREEN_WIDTH * 4 * 2, db >> 4, 0);
    ww_tile_blit(canvas, WW_SCREEN_WIDTH * 4 * 2, 1, x, y);

    video_cb((void*)canvas, WW_SCREEN_WIDTH, WW_SCREEN_HEIGHT, WW_SCREEN_WIDTH * 4 * 2);
}

void retro_deinit(void) {
}

void retro_set_controller_port_device(unsigned const port, unsigned const device) {
    (void)port;
    (void)device;
}

void retro_reset(void) {
}

size_t retro_serialize_size(void) {
    return 0;
}

bool retro_serialize(void* const data, size_t const size) {
    return false;
}

bool retro_unserialize(void const* const data, size_t const size) {
    return false;
}

void retro_cheat_reset(void) {
}

void retro_cheat_set(unsigned const a, bool const b, char const* const c) {
    (void)a;
    (void)b;
    (void)c;
}

bool retro_load_game_special(unsigned const a, struct retro_game_info const* const b, size_t const c) {
    (void)a;
    (void)b;
    (void)c;
    return false;
}

void retro_unload_game(void) {
    ww_backgrnd_destroy(&backgrnd);
    ww_filesys_destroy();
    ww_tile_destroy();
}

unsigned retro_get_region(void) {
  return RETRO_REGION_NTSC;
}
