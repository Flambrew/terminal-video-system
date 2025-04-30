#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#define RESOLUTION_12_28_UNIT 1
#define RESOLUTION_180_420_SUGGESTED 15
#define FPS_15 15
#define FPS_30 30
#define FPS_60 60

typedef struct viewport {
    uint64_t total_frames;
    uint32_t *pixels;
    uint16_t height, width;
    uint8_t fps;
    wchar_t *buf;
    clock_t start;
} Viewport;

Viewport *vp_alloc(uint8_t size, uint8_t fps);

void vp_free(Viewport *vp);

void vp_update_buffer(Viewport *vp);

void vp_print_buffer(Viewport *vp, bool final_frame);

/* True if call is made a full frame behind. */
bool vp_frame_await(Viewport *vp);