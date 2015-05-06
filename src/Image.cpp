/*******************************************************************************
 *
 * This file defines basic image processing functions
 *
 * @file Image.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <omp.h>
#include <utility>
#include <CImg.h>
#include "Image.h"

/******************************************************************************/

using namespace std;
using namespace cimg_library;

/******************************************************************************/

/**
 * Computes the luminosity of the given (R,G,B) values
 */
static float luminosity(unsigned char red, unsigned char green, unsigned char blue)
{
    return (0.212655f * (static_cast<float>(red) / 255.0f)) + 
           (0.715158f * (static_cast<float>(green) / 255.0f)) + 
           (0.072187f * (static_cast<float>(blue) / 255.0f));
}

/**
 * Fetches the specified pixel at coordinate (i,j)
 */
ImagePixel pixel(const Image& image, int i, int j)
{
    return make_tuple(image(i,j,0,0), image(i,j,0,1), image(i,j,0,2));
}

/**
 * Detects the edges in the given bitmap using the Sobel operator, producing 
 * an edge intensity map. This map is used to selectively determine where to 
 * apply antialiasing
 */
unique_ptr<float[]> edges(const Image& input, int w, int h, float& avgIntensity)
{   
    // Sobel filter in X:
    float Gx[3][3] = {{-1.0f, 0.0f, 1.0f}
                     ,{-2.0f, 0.0f, 2.0f}
                     ,{-1.0f, 0.0f, 1.0f}};
    // Sobel filter in Y:
    float Gy[3][3] = {{-1.0f, -2.0f, -1.0f}
                     ,{ 0.0f,  0.0f,  0.0f}
                     ,{ 1.0f,  2.0f,  1.0f}};

    unique_ptr<float[]> edgeMap(new float[w * h]);
    avgIntensity = 0.0f;

    #ifdef ENABLE_OPENMP
    #pragma omp parallel for
    #endif
    for (int i=1; i<(w-1); i++) {
        for (int j=1; j<(h-1); j++) {

            // Gradient values in the X and Y directions at (i,j)
            float X = 0.0f;
            float Y = 0.0f;

            for (int u=0; u<3; u++) {
                for (int v=0; v<3; v++) {

                    int ii = i + (u - 1);
                    int jj = j + (v - 1);
                    assert(ii >= 0 && ii < w);
                    assert(jj >= 0 && jj < h);

                    auto red   = input(ii, jj, 0, 0);
                    auto green = input(ii, jj, 0, 1);
                    auto blue  = input(ii, jj, 0, 2);
                    
                    float intensity = luminosity(red, green, blue);
                    X += (intensity * Gx[u][v]);
                    Y += (intensity * Gy[u][v]);
                }
            }

            // Compute the gradient magnitude and clamp to the range [0,1]:
            float magnitude = std::min(std::max(0.0f, std::sqrt(powf(X, 2.0f) + powf(Y, 2.0f))), 1.0f); 
            edgeMap[(i * h) + j]  = magnitude;

            avgIntensity += magnitude;
        }
    }

    avgIntensity /= static_cast<float>(w * h);
    
    return move(edgeMap);
}

/******************************************************************************/
