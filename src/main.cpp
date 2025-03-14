#include <graphx.h>
#include <time.h>
#include <math.h>

#include "fixedpoint.h"
#include "vector.h"
#include "color.h"

volatile Color* VRAM = (Color*)0xD40000;

const Fixed24 sphereRadius(0.5F);

const float cameraWidth = 0.25F; //Actully camera height/2 but dont worry
const Fixed24 multiplicationFactor(cameraWidth / 120.0F); //do this conversion now to save a little time later

const float horizontalFov = 126.8698976F; //this shit is NOT fov im doing the calculation wrong
const float divisionFactor(40 * tanf(3.14159F * horizontalFov / 360.0F));
const Fixed24 divisionFactorReciprocal(1.0F / divisionFactor); //do this conversion now to save a little time later

const Fixed24 distanceThreshold(0.01F);

const Vec3 cameraPos(0.25F, 0.25F, -1.0F);



//Distance estimator for raymarching
Fixed24 distanceEstimator(Vec3 pos) {
    Fixed24 dist = pos.norm() - sphereRadius;
    return dist;
}


void rayMarch(Vec3& pos, Vec3& dirNorm, Fixed24& dist, uint16_t& count, Fixed24& total){
    while (dist > distanceThreshold && count < 200) {

        //Keep the ray within (-1,-1,-1) to (1,1,1), this is how there are infinite spheres
        //While loops are faster than if statements for some reason
        while (pos.x < Fixed24(-1)) pos.x += Fixed24(2);
        while (pos.x > Fixed24(1)) pos.x -= Fixed24(2);
        while (pos.y < Fixed24(-1)) pos.y += Fixed24(2);
        while (pos.y > Fixed24(1)) pos.y -= Fixed24(2);
        while (pos.z < Fixed24(-1)) pos.z += Fixed24(2);
        while (pos.z > Fixed24(1)) pos.z -= Fixed24(2);

        //Find the distance from the ray to the nearest sphere
        dist = distanceEstimator(pos);
        if (dist < Fixed24(0)) dist = Fixed24(0); //Just making sure... but it should never be

        //March the ray forward as far as it can safely go
        pos = pos + dirNorm * dist;
     
        count++;
        total += dist;
    }
}


void genStartVectors(Vec3& pos, Vec3& dirNorm, Fixed24 _x, Fixed24 _y){
    //Ray start position on camera sensor
    //Mapping from (0 -> 320,0 -> 160) to (-4/3*cameraWidth -> 4/3*cameraWidth, -cameraWidth -> cameraWidth)
    pos.x = Fixed24(_x - Fixed24(160)) * multiplicationFactor + cameraPos.x;
    pos.y = Fixed24(Fixed24(120) - _y) * multiplicationFactor + cameraPos.y;
    pos.z = cameraPos.z;

    //Direction vector of the ray
    Vec3 dir;
    dir.x = Fixed24(_x - Fixed24(160)) * divisionFactorReciprocal;
    dir.y = Fixed24(Fixed24(120) - _y) * divisionFactorReciprocal;
    dir.z = Fixed24(1);

    //Normalized direction vector
    Fixed24 len;
    len = dir.norm();
    dirNorm = dir * div(Fixed24(1), len);
}


void getRGB(int& r, int& g, int& b, Fixed24 dist, Fixed24 totalDist, Vec3 pos, Vec3 dirNorm, uint16_t count) {
    Fixed24 lightness(0);
    if (dist <= distanceThreshold) {
        //Normalize position vector
        //length = rayPos.norm();
        //rayPosNorm = rayPos * div(Fixed24(1), length);
        //Since the ray is touching a sphere at 0,0,0 with radius 0.5, the ray's length is 0.5, just *2 to get normal
        //generalize this for different size spheres later
        Vec3 posNorm = pos * Fixed24(2);

        //Dot product of position and direction will give the cos of the angle the sphere was hit at (if you really think about it)
        Fixed24 dotProduct = dot(posNorm, dirNorm);
        if (dotProduct < Fixed24(0)) dotProduct = -dotProduct;
        if (dotProduct > Fixed24(1)) dotProduct = Fixed24(1); //Should never be more than 1 because the vectors are normalized but just making sure

        //Inverse square root of distance for lighting
        Fixed24 invSqrt;
        if (totalDist < Fixed24(1)) {
            invSqrt = 1;
        }
        else {
            invSqrt = div(Fixed24(1), sqrt(totalDist));
        }


        //Start with white, darken based on inverse square root and angle of incidence
        lightness = Fixed24(31) * invSqrt * dotProduct;
        int24_t lightnessInt = lightness.floor();


        if (lightnessInt > 31) lightnessInt = 31; //Haha.. just to be careful
        g = lightnessInt + (count >> 3);
        if (g > 31) g = 31;
        r = lightnessInt;
        b = lightnessInt;

    }
    else {
        g = 31;
        r = 0;
        b = 0;

    }
}


int main(void)
{

    // Start the timer
    clock_t start = clock();

    //Initializing variables
    Vec3 rayPos; //Ray position
    Fixed24 length;  //Used for normalizing
    Vec3 rayDirNorm;  //Normalized direction vector of ray 
    Fixed24 distance;  //Distance from ray to nearest object
    uint16_t counter; //Number of times the ray has marched
    Fixed24 totalDistance; //Total distance the ray has marched 
    Vec3 rayPosNorm; //Normalized position vector of the ray
    Fixed24 dotProd; //Dot product for lighting
    Fixed24 inverseSqrt; //Inverse square root for lighting
    Fixed24 lightness;
    int24_t r;
    int24_t b;
    int24_t g;


    //-----Main render loop

    //Iterate over every pixel
    for (int24_t x = 0; x < 320; x++) {
        for (int24_t y = 0; y < 240; y++) {

            genStartVectors(rayPos, rayDirNorm, Fixed24(x), Fixed24(y));
            
            distance = Fixed24(1);
            counter = 0;
            totalDistance = Fixed24(0);
            rayMarch(rayPos, rayDirNorm, distance, counter, totalDistance);

            getRGB(r, g, b, distance, totalDistance, rayPos, rayDirNorm, counter);
            
            Color a = fromRGB(r, g, b);
           
            VRAM[x + 320 * y] = a;

        }
    }

    
    //Stop the timer
    clock_t end = clock();

    //Calculate the elapsed time in seconds
    int elapsed_time = (int)((end - start) / CLOCKS_PER_SEC);

    //Waits for a key
    while (!os_GetCSC());

    gfx_Begin();
    gfx_SetColor(0);
    //Print elapsed time
    gfx_SetTextXY(0, 210);
    gfx_PrintInt(elapsed_time, 5);

    //Waits for a key
    while (!os_GetCSC());

    //End graphics drawing
    gfx_End();

    return 0;
}
