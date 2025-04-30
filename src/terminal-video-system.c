#include <locale.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
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
#define RGB6X 23
#define RGBL 11
#define RGB 10
#define BW 2

static const int FG_COLOR = 30;
static const int BG_COLOR = 40;
static const int BR_COLOR = 60;
static const int COLOR_RESET = 9;
static const int PIXEL_THRESHOLD = 32;
static const int COLOR_THRESHOLD = 128;

static const int ASPECT_Y = 3;
static const int ASPECT_X = 7;
static const int PIXEL_CLUSTER_DIM = 4;
static const int PIXELS_PER_CLUSTER = 8;

static const int BRAILLE_MAP[] = {0, 3, 1, 4, 2, 5, 6, 7};

static const int PSEUDODECIMAL = 1000000;

typedef struct viewport {
    uint64_t total_frames;
    uint32_t *bitmap;
    uint16_t height, width;
    uint8_t fps, color_depth;
    wchar_t *buf;
    clock_t start;
} Viewport;

Viewport *vp_alloc(uint8_t size, uint8_t fps, uint8_t color_depth) {
    setlocale(LC_CTYPE, "");

    Viewport *vp;
    vp = (Viewport *) malloc(sizeof(Viewport));

    vp->height = size * PIXEL_CLUSTER_DIM * ASPECT_Y;
    vp->width = size * PIXEL_CLUSTER_DIM * ASPECT_X;
    vp->start = vp->total_frames = 0;
    vp->color_depth = color_depth; // value is chars per cluster
    vp->fps = fps;

    vp->bitmap = (uint32_t *) malloc(vp->height * vp->width * sizeof(uint32_t));
    vp->buf = (wchar_t *) malloc(vp->height * vp->width * vp->color_depth * sizeof(wchar_t)); // oversize
    return vp;
}

void vp_free(Viewport *vp) {
    printf("\e[?25h");
    free(vp->bitmap);
    free(vp->buf);
    free(vp);
}

void vp_update_buffer(Viewport *vp) {
    wchar_t c;
    uint64_t k;
    uint32_t pix;
    uint16_t i, j, r, g, b;
    uint8_t x, y, color;
    for (i = k = 0; i < vp->height; i += 4) {
        for (j = 0; j < vp->width; j += 2) {

            c = 0x2800;
            r = g = b = 0;
            for (y = 0; y < 4; ++y) {
                for (x = 0; x < 2; ++x) {
                    pix = vp->bitmap[(i + y) * vp->width + j + x];
                    r += (pix >> 16) & 0xFF;
                    g += (pix >> 8) & 0xFF;
                    b += (pix) & 0xFF;

                    if ((((pix >> 16) & 0xFF) + ((pix >> 8) & 0xFF) + (pix & 0xFF)) / 3 > PIXEL_THRESHOLD) {
                        c |= (1 << BRAILLE_MAP[y * 2 + x]);
                    }
                }
            }

            r /= PIXELS_PER_CLUSTER; g /= PIXELS_PER_CLUSTER; b /= PIXELS_PER_CLUSTER; 
            switch (vp->color_depth) {
                case RGB256X:
                    k += swprintf(vp->buf + k, vp->color_depth, L"\e[38;2;%d;%d;%dm\e[48;5;16m%lc", r, g, b, c);
                    break;
                case RGB6X:
                    color = r * 27 / 32 + g * 9 / 64 + b * 3 / 128 + 16;
                    k += swprintf(vp->buf + k, vp->color_depth, L"\e[38;5;%dm\e[48;5;16m%lc", color, c);
                    break;
                case RGBL:
                    color = ((r > COLOR_THRESHOLD) << 2) + ((g > COLOR_THRESHOLD) << 1) + (b > COLOR_THRESHOLD);
                    color += (r + g + b) / 3 > COLOR_THRESHOLD ? BR_COLOR : 0;
                    k += swprintf(vp->buf + k, vp->color_depth, L"\e[%d;%dm%lc", FG_COLOR + color, BG_COLOR, c);
                    break;
                case RGB:
                    color = ((r > COLOR_THRESHOLD) << 2) + ((g > COLOR_THRESHOLD) << 1) + (b > COLOR_THRESHOLD);
                    k += swprintf(vp->buf + k, vp->color_depth, L"\e[%d;%dm%lc", FG_COLOR + color, BG_COLOR, c);
                    break;
                case BW: default:
                    k += swprintf(vp->buf + k, vp->color_depth, L"%lc", c);
                    break;
            }
        }

        k += swprintf(vp->buf + k, RGB, L"\e[%d;%dm\n", FG_COLOR + COLOR_RESET, BG_COLOR + COLOR_RESET);
    }
}

void vp_print_buffer(Viewport *vp, bool final_frame) {
    printf("\e[?25l%ls\e[%dA", vp->buf, vp->height / 4);
    if (final_frame) {
        printf("%ls\e[?25h", vp->buf);
    }
}

bool vp_frame_await(Viewport *vp) {
    clock_t current, target;
    if (vp->start == 0) {
        vp->start = clock();
    } else {
        target = vp->start + PSEUDODECIMAL * CLOCKS_PER_SEC / vp->fps * vp->total_frames / PSEUDODECIMAL;
        while ((current = clock()) < target);
    }

    vp->total_frames++;
    return current - target > 1000 / vp->fps;
}

uint64_t vp_pause(Viewport *vp) {
    uint64_t count;
    count = vp->total_frames;
    vp->total_frames = 0;
    return count;
}