#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#define RESOLUTION_SUGGESTED 15
#define RESOLUTION_UNIT_12x28 1
#define FPS_ULTRA 144
#define FPS_HIGH 60
#define FPS_MID 30
#define FPS_LO 15

typedef struct viewport {
    uint64_t total_frames;
    uint32_t *bitmap;
    uint16_t height, width;
    uint8_t fps;
    wchar_t *buf;
    clock_t start;
} Viewport;

/* Allocate and instantiate a new instance of the video system. */
Viewport *vp_alloc(uint8_t size, uint8_t fps);

/* Free an existing instance of the video system. Performs cleanup. */
void vp_free(Viewport *vp);

/* Update image buffer to current state of bitmap. */
void vp_update_buffer(Viewport *vp);

/* Print image buffer to terminal. Resets cursor position to top of buffer unless final frame. */
void vp_print_buffer(Viewport *vp, bool final_frame);

/* True if call is made a full frame behind. */
bool vp_frame_await(Viewport *vp);