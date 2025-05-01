#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "include/display.h"

// -----=====<<<<< TESTING >>>>>=====----- //

static void draw(Viewport *vp, uint64_t k) {
    uint64_t i, j;
    for (i = 0; i < vp->height; ++i) {
        for (j = 0; j < vp->width; ++j) {
            vp->bitmap[i * vp->width + j] = ((i+j+k)%256 << 16) + ((i+(vp->width-j)+k)%256 << 8) + ((vp->height-i)+j+k)%256; 
        }
    }
}

int main() {
    Viewport *vp = vp_alloc(RESOLUTION_LARGE, FPS_ULTRA, RGB);

    int64_t k, s, n, p, frames, framelen;
    s = p = 0;
    frames = 342;//vp->fps * 5;
    framelen = 1000 / vp->fps;
    for (k = frames - 1; k >= 0; --k) {
        //draw(vp, k);
        char path[25];
        path[0]='s'; 
        path[1]='r'; 
        path[2]='c'; 
        path[3]='/'; 
        path[4]='t'; 
        path[5]='c'; 
        path[6]='a'; 
        path[7]='t'; 
        path[8]='/';
        path[9]='0'; 
        path[10]='0'; 
        path[11]=(frames-k)/100+'0'; 
        path[12]=(frames-k)/10%10+'0'; 
        path[13]=(frames-k)%10+'0'; 
        path[14]='.'; 
        path[15]='b'; 
        path[16]='m'; 
        path[17]='p'; 
        path[18]='\0'; 
        vp_load_bmp(vp, path);
        vp_update_buffer(vp);
        n = vp_frame_await(vp);
        vp_print_buffer(vp, k == 0);
        if (k != vp->fps * 5 - 1) {
            if (n > p) {
                p = n;
            }
            s += n;
        }
    }

    printf("[Display] Resolution: %dx%dpx, Color: %s, Framerate: %dfps; [Latency] Average: %dms=%dfr, Peak: %dms=%dfr\n",
            vp->height, vp->width, vp->color_depth == BW ? "BW" : vp->color_depth == RGB ? "RGB" : vp->color_depth == RGBL ? "RGBL" :
            vp->color_depth == BW24X ? "BW24X" : vp->color_depth == RGB6X ? "RGB6X" : "RGB256X" , vp->fps, (int) (s / frames),
            (int) (s / frames / framelen), (int) p, (int) (p / framelen));

    vp_free(vp);
    return 0;
}