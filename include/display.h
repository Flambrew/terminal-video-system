#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#define RESOLUTION_LARGE 15
#define RESOLUTION_MEDIUM 10
#define RESOLUTION_SMALL 5
#define RESOLUTION_MIN 1
#define FPS_ULTRA 144
#define FPS_HIGH 60
#define FPS_MID 30
#define FPS_LOW 15
#define RGB256X 31
#define RGB6X 24
#define BW24X 23
#define RGBL 11
#define RGB 10
#define BW 2

typedef struct viewport {
    uint64_t total_frames;
    uint32_t *bitmap;
    uint16_t height, width;
    uint8_t fps, color_depth;
    wchar_t *buf;
    clock_t start;
} Viewport;

/* Allocate and instantiate a new instance of the video system. */
Viewport *vp_alloc(uint8_t size, uint8_t fps, uint8_t color_depth);

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