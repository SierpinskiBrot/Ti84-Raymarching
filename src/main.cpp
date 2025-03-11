#include <graphx.h>
#include <time.h>

#include "fixedpoint.h"
#include "vector.h"

//volatile uint16_t* VRAM = (uint16_t*)0xD40000;

float sphereRadius = 0.5F;

//Distance estimator for raymarching
Fixed24 distanceEstimator(Vec3 pos) {
    Fixed24 dist = pos.norm() - Fixed24(0.5F);
    return dist;
}


int main(void)
{
    //Start gfx
    gfx_Begin();
    
    //Create the pallate of grey
    for (int p = 0; p < 255; p++) {
        gfx_palette[p] = gfx_RGBTo1555(p, p, p);
    }

    //Clear the screen
    gfx_FillScreen(0);

    // Start the timer
    clock_t start = clock();

    //Initializing variables
    Vec3 rayPos; //Ray position
    Vec3 rayDir; //Direction vector of ray
    Fixed24 length;  //Used for normalizing
    Vec3 rayDirNorm;  //Normalized direction vector of ray 
    Fixed24 distance;  //Distance from ray to nearest object
    uint24_t counter; //Number of times the ray has marched
    Fixed24 totalDistance; //Total distance the ray has marched 
    Vec3 rayPosNorm; //Normalized position vector of the ray
    Fixed24 dotProd; //Dot product for lighting
    Fixed24 inverseSqrt; //Inverse square root for lighting
    Fixed24 lightness;
    float cameraWidth = 0.25; //Actully camera height/2 but dont worry


    //-----Main render loop

    //Iterate over every pixel
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++) {

            

            //Ray start position on camera sensor
            //Mapping from (0 -> 320,0 -> 160) to (-4/3*cameraWidth -> 4/3*cameraWidth, -cameraWidth -> cameraWidth)
            rayPos.x = Fixed24((x - 160) / (120.0F / cameraWidth) + 0.25F);
            rayPos.y = Fixed24((120 - y) / (120.0F / cameraWidth) + 0.25F);
            rayPos.z = Fixed24(-1);

            //Direction vector of the ray
            rayDir.x = Fixed24((x - 160) / 80.0F);
            rayDir.y = Fixed24((120 - y) / 80.0F);
            rayDir.z = Fixed24(1);


            //Normalized direction vector
            length = rayDir.norm();
            rayDirNorm = rayDir * div(Fixed24(1), length);


            distance = Fixed24(1);
            counter = 0;
            totalDistance = Fixed24(0);
            //March forward 500 times
            while (distance > Fixed24(0.01F) && counter < 500) {
        
                //Keep the ray within (-1,-1,-1) to (1,1,1), this is how there are infinite spheres
                while (rayPos.x < Fixed24(-1)) rayPos.x += Fixed24(2);
                while (rayPos.x > Fixed24(1)) rayPos.x -= Fixed24(2);
                while (rayPos.y < Fixed24(-1)) rayPos.y += Fixed24(2);
                while (rayPos.y > Fixed24(1)) rayPos.y -= Fixed24(2);
                while (rayPos.z < Fixed24(-1)) rayPos.z += Fixed24(2);
                while (rayPos.z > Fixed24(1)) rayPos.z -= Fixed24(2);


                //Find the distance from the ray to the nearest sphere
                distance = distanceEstimator(rayPos);
                //if (dist < 0) dist = -dist; //Just making sure... but it should never be

                //March the ray forward as far as it can safely go
                rayPos = rayPos + rayDirNorm * distance;

                counter++;
                totalDistance += distance;
            }

            gfx_SetColor(0); //Color defaults to black if no circle was hit

            if (distance <= Fixed24(0.01F)) {
                //Normalize position vector
                length = rayPos.norm();
                rayPosNorm = rayPos * div(Fixed24(1), length);

                //Dot product of position and direction will give the cos of the angle the sphere was hit at (if you really think about it)
                dotProd = dot(rayPosNorm, rayDirNorm);
                if (dotProd < Fixed24(0)) dotProd = -dotProd;
                if (dotProd > Fixed24(1)) dotProd = Fixed24(1); //Should never be more than 1 because the vectors are normalized but just making sure

                //Inverse square root of distance for lighting
                inverseSqrt = div(Fixed24(1), sqrt(totalDistance));
                if (inverseSqrt > Fixed24(1)) inverseSqrt = Fixed24(1); //This one can actually be greater than one
                

                //Start with white, darken based on inverse square root and angle of incidence
                //g = 255*invSqrt*dot1;
                lightness = Fixed24(255) * inverseSqrt * dotProd;
                if (lightness > Fixed24(255)) lightness = Fixed24(255); //Haha.. just to be careful
                    
                gfx_SetColor(lightness.floor());
            }
            gfx_SetPixel(x,y);
        }
    }

    //Stop the timer
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
