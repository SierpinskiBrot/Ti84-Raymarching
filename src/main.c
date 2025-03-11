#include <ti/getcsc.h>
#include <sys/util.h>
#include <stdlib.h>
#include <graphx.h>
#include <time.h>
#include <math.h>
#include <tice.h>

volatile uint16_t* VRAM = (uint16_t*)0xD40000;

float sphereRadius = 0.5;

float distanceEstimator(float p[3]) {
    float dist;
    dist = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]) - sphereRadius;
    return dist;
}


int main(void)
{
    //Stary gfx
    gfx_Begin();

    //Create the pallate of grey
    for (int p = 0; p < 255; p++) {
        gfx_palette[p] = gfx_RGBTo1555(p, p, p);
    }

    //Clear the screen
    gfx_FillScreen(0);

    // Start the clock
    clock_t start = clock();

    //Initializing variables
    float pos[3]; //Ray position
    float dir[3]; //Direction vector of ray 
    float len;  //Used for normalizing
    float dirNorm[3]; //Normalized direction vector of ray 
    float dist; //Distance from ray to nearest object
    int counter; //Number of times the ray has marched
    float totalDist; //Total distance the ray has marched
    float posNorm[3]; //Normalized position vector of the ray 
    float dot; //Dot product for lighting
    float invSqrt; //Inverse square root for lighting
    float r; 
    float g;
    float b;
    float cameraWidth = 0.25; //Actully camera height/2 but dont worry

    //-----Main render loop

    //Iterate over every pixel
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {

            

            //Ray start position on camera sensor
            //Mapping from (0 -> 320,0 -> 160) to (-4/3*cameraWidth -> 4/3*cameraWidth, -cameraWidth -> cameraWidth)
            pos[0] = (x - 160) / (120.0 / cameraWidth) + 0.25;
            pos[1] = (120 - y) / (120.0 / cameraWidth) + 0.25;
            pos[2] = - 1;

            //Direction vector of the ray
            //Using 80 gives ~127 degree horizontal fov
            dir[0] = (x - 160) / 80.0;
            dir[1] = (120 - y) / 80.0;
            dir[2] = 1.0;

            //Normalized direction vector
            len = sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
            dirNorm[0] = dir[0] / len;
            dirNorm[1] = dir[1] / len;
            dirNorm[2] = dir[2] / len;


            
            dist = 1.0;
            counter = 0;
            totalDist = 0.0;
            //March forward 500 times
            while (dist > 0.01 && counter < 500) {
        
                //Keep the ray within (-1,-1,-1) to (1,1,1), this is how there are infinite spheres
                while (pos[0] < -1) pos[0] += 2;
                while (pos[0] > 1) pos[0] -= 2;
                while (pos[1] < -1) pos[1] += 2;
                while (pos[1] > 1) pos[1] -= 2;
                while (pos[2] < -1) pos[2] += 2;
                while (pos[2] > 1) pos[2] -= 2;

                //Find the distance from the ray to the nearest sphere
                dist = distanceEstimator(pos);
                if (dist < 0) dist = -dist; //Just making sure... but it should never be

                //March the ray forward as far as it can safely go
                pos[0] += dist * dirNorm[0];
                pos[1] += dist * dirNorm[1];
                pos[2] += dist * dirNorm[2];

                counter++;
                totalDist += dist;
            }

            gfx_SetColor(0); //Color defaults to black if no circle was hit

            if (dist <= 0.01) {
                //Normalize position vector
                len = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
                posNorm[0] = pos[0] / len;
                posNorm[1] = pos[1] / len;
                posNorm[2] = pos[2] / len;

                //Dot product of position and direction will give the cos of the angle the sphere was hit at (if you really think about it)
                dot = posNorm[0] * dirNorm[0] + posNorm[1] * dirNorm[1] + posNorm[2] * dirNorm[2];
                if (dot < 0) dot = -dot;
                if (dot > 1) dot = 1; //Should never be more than 1 because the vectors are normalized but just making sure

                //Inverse square root of distance for lighting
                invSqrt = 1 / sqrt(totalDist);
                if (invSqrt > 1) invSqrt = 1; //This one can actually be greater than one

                //Start with white, darken based on inverse square root and angle of incidence
                g = 255*invSqrt*dot;

                if (g > 255) g = 254; //Haha.. just to be careful
                    
                gfx_SetColor(g);
            }
            gfx_SetPixel(x,y);
        }
    }

    //Stop the clock
    clock_t end = clock();

    //Calculate the elapsed time in seconds
    int elapsed_time = (int)((end - start) / CLOCKS_PER_SEC);
    //Print elapsed time
    gfx_SetTextXY(0, 210);
    gfx_PrintInt(elapsed_time, 5);
    gfx_SwapDraw();

    //Waits for a key
    while (!os_GetCSC());

    //End graphics drawing
    gfx_End();

    return 0;
}
