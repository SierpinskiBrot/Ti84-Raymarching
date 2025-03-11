#include <ti/getcsc.h>
#include <sys/util.h>
#include <stdlib.h>
#include <graphx.h>
#include <time.h>
#include <math.h>
#include <tice.h>

volatile uint16_t* VRAM = (uint16_t*)0xD40000;

float sphereCenter[3] = { 0,0,0 };
float sphereRadius = 0.5;

float distanceEstimator(float p[3]) {
    float dist;
    dist = sqrt(
        (p[0] - sphereCenter[0]) * (p[0] - sphereCenter[0]) +
        (p[1] - sphereCenter[1]) * (p[1] - sphereCenter[1]) +
        (p[2] - sphereCenter[2]) * (p[2] - sphereCenter[2])) - sphereRadius;
    return dist;
}


int main(void)
{
    
    gfx_Begin();

    for (int p = 0; p < 255; p++) {
        gfx_palette[p] = gfx_RGBTo1555(p, p, p);
    }

    /* Clear the screen */
    gfx_FillScreen(0);

    // Start the clock
    clock_t start = clock();
    float pos[3];
    float dir[3];
    float len;
    float dirNorm[3];
    float dist;
    int counter;
    float totalDist;
    float posNorm[3];
    float dot;
    float invSqrt;
    float r;
    float g;
    float b;

    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {

            pos[0] = (x - 160) / 480.0 + 0.25;
            pos[1] = (120 - y) / 480.0 + 0.25;
            pos[2] = - 1;

            dir[0] = (x - 160) / 80.0;
            dir[1] = (120 - y) / 80.0;
            dir[2] = 1.0;

            len = sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
            dirNorm[0] = dir[0] / len;
            dirNorm[1] = dir[1] / len;
            dirNorm[2] = dir[2] / len;

            dist = 1.0;
            counter = 0;
            totalDist = 0.0;
            while (dist > 0.01 && counter < 500) {
        
                while (pos[0] < -1) pos[0] += 2;
                while (pos[0] > 1) pos[0] -= 2;
                while (pos[1] < -1) pos[1] += 2;
                while (pos[1] > 1) pos[1] -= 2;
                while (pos[2] < -1) pos[2] += 2;
                while (pos[2] > 1) pos[2] -= 2;

                dist = distanceEstimator(pos);
                if (dist < 0) dist = -dist;

                pos[0] += dist * dirNorm[0];
                pos[1] += dist * dirNorm[1];
                pos[2] += dist * dirNorm[2];

                counter++;
                totalDist += dist;
            }
            gfx_SetColor(0);
            if (dist <= 0.01) {

                len = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
                posNorm[0] = pos[0] / len;
                posNorm[1] = pos[1] / len;
                posNorm[2] = pos[2] / len;

                dot = posNorm[0] * dirNorm[0] + posNorm[1] * dirNorm[1] + posNorm[2] * dirNorm[2];
                if (dot < 0) dot = -dot;
                if (dot > 1) dot = 1;

                invSqrt = 1 / sqrt(totalDist);
                if (invSqrt > 1) invSqrt = 1;

                g = 255*invSqrt*dot;

                if (g > 255) g = 254;
                    
                gfx_SetColor(g);
            }
            /*
            else {
                gfx_SetColor(gfx_RGBTo1555((int)255, counter, 0));
            }
            */

            
            gfx_SetPixel(x,y);

        }
    }

    // Stop the clock
    clock_t end = clock();

    // Calculate the elapsed time in seconds
    int elapsed_time = (int)((end - start) / CLOCKS_PER_SEC);

    gfx_SetTextXY(0, 210);
    gfx_PrintInt(elapsed_time, 5);
    gfx_SwapDraw();

    /* Waits for a key */
    while (!os_GetCSC());

    /* End graphics drawing */
    gfx_End();

    return 0;
}
