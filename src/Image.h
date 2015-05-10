/*******************************************************************************
 *
 * This file defines basic image processing functions
 *
 * @file Image.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef IMAGE_H
#define IMAGE_H

#define cimg_display 0
#include <CImg.h>
#include <tuple>
#include <memory>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

/******************************************************************************/

/**
 * Generalized image type
 */
typedef cimg_library::CImg<unsigned char> Image;

/**
 * Definition of a pixel in the given Image type
 */
typedef std::tuple<unsigned char, unsigned char, unsigned char> ImagePixel;

#define RED(pixel)   (get<0>((pixel)))
#define GREEN(pixel) (get<1>((pixel)))
#define BLUE(pixel)  (get<2>((pixel)))

/******************************************************************************/

/**
 * Fetches the specified pixel at coordinate (i,j)
 */
ImagePixel pixel(const Image& image, int i, int j);

/**
 * Detects the edges in the given bitmap using the Sobel operator, producing 
 * an edge intensity map. This map is used to selectively determine where to 
 * apply antialiasing
 */
std::unique_ptr<float[]> edges(const Image& input, int w, int h, float& avgIntensity);

/******************************************************************************/

#endif
