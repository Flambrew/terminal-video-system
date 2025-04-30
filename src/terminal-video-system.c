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
#define FPS_LO 15
#define RGBALL 30
#define RGB_6 22
#define RGBL 11
#define RGB 10
#define BW 1

static const int FG_COLOR = 30;
static const int BG_COLOR = 40;
static const int BR_COLOR = 60;
static const int COLOR_RESET = 9;
static const int PIXEL_THRESHOLD = 256;
static const int COLOR_THRESHOLD = 128;

static const int ASPECT_Y = 3;
static const int ASPECT_X = 7;
static const int PIXEL_CLUSTER_DIM = 4;
static const int PIXELS_PER_CLUSTER = 8;

static const int BRAILLE_MAP[] = {0, 3, 1, 4, 2, 5, 6, 7};

static const int PSEUDODECIMAL = 1000;

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
    vp->buf = (wchar_t *) malloc(((vp->height * vp->width / PIXELS_PER_CLUSTER + 1) * vp->color_depth + vp->height + 1) * sizeof(wchar_t));
    return vp;
}

void vp_free(Viewport *vp) {
    printf("\e[?25h");
    free(vp->bitmap);
    free(vp->buf);
    free(vp);
}

static void vp_buf_append(Viewport *vp, uint64_t *k, uint8_t r, uint8_t g, uint8_t b, wchar_t c) {
    uint8_t color;
    switch (vp->color_depth) {
        case BW:
            *k += swprintf(vp->buf + *k, vp->color_depth, L"%lc", c);
            break;
        case RGB:
            color = ((r > COLOR_THRESHOLD) << 2) + ((g > COLOR_THRESHOLD) << 1) + (b > COLOR_THRESHOLD);
            *k += swprintf(vp->buf + *k, vp->color_depth, L"\e[%d;%dm%lc", FG_COLOR + color, BG_COLOR, c);
            break;
        case RGBL:
            color = ((r > COLOR_THRESHOLD) << 2) + ((g > COLOR_THRESHOLD) << 1) + (b > COLOR_THRESHOLD);
            color += (r + g + b) / 3 > COLOR_THRESHOLD ? BR_COLOR : 0;
            *k += swprintf(vp->buf + *k, vp->color_depth, L"\e[%d;%dm%lc", FG_COLOR + color, BG_COLOR, c);
            break;
        case RGB_6:
            *k += swprintf(vp->buf + *k, vp->color_depth, L"\e[38;5;%dm\e[48;5;16m%lc", 16 + 36 * r + 6 * g + b, c);
            break;
        case RGBALL:
            *k += swprintf(vp->buf + *k, vp->color_depth, L"\e[38;2;%d;%d;%dm\e[48;5;16m%lc", r, g, b, c);
            break;
    }
}

void vp_update_buffer(Viewport *vp) {
    wchar_t c;
    uint64_t k;
    uint32_t pix;
    uint16_t i, j, r, g, b;
    uint8_t x, y;
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

                    if ((((pix >> 16) & 0xFF) + ((pix >> 8) & 0xFF) + (pix & 0xFF)) / 3 < PIXEL_THRESHOLD) {
                        c |= (1 << BRAILLE_MAP[y * 2 + x]);
                    }
                }
            }
            vp_buf_append(vp, &k, r / PIXELS_PER_CLUSTER, g / PIXELS_PER_CLUSTER, b / PIXELS_PER_CLUSTER, c);
        }
        k += swprintf(vp->buf + k, vp->color_depth, L"\e[%d;%dm\n", FG_COLOR + COLOR_RESET, BG_COLOR + COLOR_RESET);
    }

    vp->buf[k] = '\0';
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
