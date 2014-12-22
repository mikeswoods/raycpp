/******************************************************************************
 *
 * This file defines a basic RGB color type where each component is represented
 * by floating point values bounded to the range [0,1]
 *
 * @file Color.h
 * @author Michael Woods
 *
 ******************************************************************************/

#ifndef COLOR_H
#define COLOR_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include "Utils.h"

/*****************************************************************************/

class Color
{
    protected:
        float r, g, b;

    public:
		const static Color DEBUG;
        const static Color BLACK;
        const static Color WHITE;
        const static Color RED;
        const static Color GREEN;
        const static Color BLUE;

        static Color fromHSV(float h, float s, float v);
		
		// Average an array of colors
		static Color average(Color * const colors, unsigned int n);

        Color();
        Color(int r, int g, int b);
		Color(int rgb[3]);
        Color(float r, float g, float b);
		Color(float rgb[3]);
        Color(const Color& other);

        void setR(float r);
        void setR(int r);
        void setG(float g);
        void setG(int g);
        void setB(float b);
        void setB(int b);

        // Red as a float in [0,1]
        float fR() const { return this->r; }
        // Green as a float in [0,1]
        float fG() const { return this->g; }
        // Blue as a float in [0,1]
        float fB() const { return this->b; }

        // Red as an int in [0,255]
        unsigned char iR() const { return (unsigned char)floor(this->r * 255.0f); }
        // Green as an int in [0,255]
        unsigned char iG() const { return (unsigned char)floor(this->g * 255.0f); }
        // Blue as an int in [0,255] 
        unsigned char iB() const { return (unsigned char)floor(this->b * 255.0f); }

		// Returns the luminosity of the color in the range [0,1]
		float luminosity() const;

		// Returns the hue-saturation-value of the color
        void toHSV(float* h, float* s, float* v) const;

        Color& operator+=(const Color &c);
        Color& operator-=(const Color &c);
		Color& operator*=(float scale);
		Color& operator*=(int scale);
		Color& operator/=(float scale);

        friend std::ostream& operator<<(std::ostream& s, const Color& color);
        friend bool operator==(const Color& c1, const Color& c2);
        friend bool operator!=(const Color& c1, const Color& c2);
        friend const Color operator+(const Color& c1, const Color& c2);
        friend const Color operator*(const Color& c1, const Color& c2);
        friend const Color operator*(const Color& c, const float scale);
		friend const Color operator*(const float scale, const Color& c);
        friend const Color operator/(const Color& c, const float scale);
};

/*****************************************************************************/

#endif
