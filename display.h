#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

typedef enum resolution {
    RES_LARGE = 15,
    RES_MEDIUM = 10,
    RES_SMALL = 5,
    RES_MIN = 1
} Resolution;

typedef enum framerate {
    FPS_ULTRA = 144,
    FPS_HIGH = 60,
    FPS_MID = 30,
    FPS_LOW = 15,
    FPS_STEP = 1
} Framerate;

typedef enum color_mode {
    MODE_RGB256X = 31,
    MODE_RGB6X = 24,
    MODE_BW24X = 23,
    MODE_RGBL = 11,
    MODE_RGB = 10,
    MODE_BW = 2
} Color_Mode;

typedef struct viewport {
    uint64_t total_frames;
    uint32_t *bitmap;
    uint16_t height, width;
    uint8_t fps, color_depth;
    wchar_t *buf;
    clock_t start;
} Viewport;

/* Allocate and instantiate a new instance of the video system. */
Viewport *vp_alloc(Resolution size, Framerate fps, Color_Mode mode);

/* Free an existing instance of the video system. Performs cleanup. */
void vp_free(Viewport *vp);

/* Load and scale BMP image as current frame. */
void vp_load_bmp(Viewport *vp, char *path);

/* Update image buffer to current state of bitmap. */
void vp_update_buffer(Viewport *vp);

/* Print image buffer to terminal. Resets cursor position to top of buffer unless final frame. */
void vp_print_buffer(Viewport *vp, bool final_frame);

/* True if call is made a full frame behind. */
clock_t vp_frame_await(Viewport *vp);

/* Stop the frame timer, return total frames since previous pause. */
uint64_t vp_pause(Viewport *vp);