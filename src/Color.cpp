/******************************************************************************
 *
 * This file defines a basic RGB color type where each component is represented
 * by floating point values bounded to the range [0,1]
 *
 * @file Color.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <cmath>
#include <iostream>
#include "Color.h"

using namespace std;

/*****************************************************************************/

Color::Color() :
	r(0.0f),
	g(0.0f),
	b(0.0f)
{ 
	
}

Color::Color(int r, int g, int b)
{ 
    this->setR(r);
    this->setG(g);
    this->setB(b);
}

Color::Color(int rgb[3])
{ 
    this->setR(rgb[0]);
    this->setG(rgb[1]);
    this->setB(rgb[2]);
}

Color::Color(float _r, float _g, float _b) : 
    r(Utils::unitClamp(_r)), 
    g(Utils::unitClamp(_g)), 
    b(Utils::unitClamp(_b))
{ 
	
}

Color::Color(float rgb[3]) : 
    r(Utils::unitClamp(rgb[0])), 
    g(Utils::unitClamp(rgb[1])), 
    b(Utils::unitClamp(rgb[2]))
{ 
	
}

Color::Color(const Color& other)
{
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
}

// Predefined colors:
const Color Color::DEBUG = Color(1.0f, 0.0f, 1.0f);
const Color Color::BLACK = Color(0.0f, 0.0f, 0.0f);
const Color Color::WHITE = Color(1.0f, 1.0f, 1.0f);
const Color Color::RED   = Color(1.0f, 0.0f, 0.0f);
const Color Color::GREEN = Color(0.0f, 1.0f, 0.0f);
const Color Color::BLUE  = Color(0.0f, 0.0f, 1.0f);

/******************************************************************************
 * Courtesy of "Color Conversion Algorithms"
 * http://www.cs.rit.edu/~ncs/color/t_convert.html
 * 
 * h = Hue: [0,360] degrees
 * s = Saturation: [0,1] 
 * v = Brightness: [0,1]
 *****************************************************************************/
Color Color::fromHSV(float h, float s, float v)
{
    int i;
    float f, p, q, t;
    float r = 0.0f, g = 0.0f, b = 0.0f;

    h = fmod(h, 360.f);
    s = Utils::unitClamp(s);
    v = Utils::unitClamp(v);

    h /= 60;
    i = (int)floor(h);
    f = h - i;
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));
    switch(i) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;
        case 1:
            r = q;
            g = v;
            b = p;
            break;
        case 2:
            r = p;
            g = v;
            b = t;
            break;
        case 3:
            r = p;
            g = q;
            b = v;
            break;
        case 4:
            r = t;
            g = p;
            b = v;
            break;
        case 5:
        default: // case 5:
            r = v;
            g = p;
            b = q;
            break;
    }

    return Color(r, g, b);
}

/******************************************************************************
 * Average an array of colors
 *****************************************************************************/
Color Color::average(Color * const colors, unsigned int n)
{
	float rc = 0.0f, gc = 0.0f, bc = 0.0f;
	float N  = static_cast<float>(n);

	for (unsigned int i=0; i<n; i++) {
		rc += colors[i].fR();
		gc += colors[i].fG();
		bc += colors[i].fB();
	}

	return Color(rc / N, gc / N, bc / N);
}

void Color::setR(float _r) 
{
    this->r = Utils::unitClamp(_r);
}

void Color::setG(float _g) 
{
    this->g = Utils::unitClamp(_g); 
}

void Color::setB(float _b) 
{
    this->b = Utils::unitClamp(_b);
}

void Color::setR(int _r) 
{
    this->r = Utils::unitClamp(static_cast<float>(_r) / 255.0f);
}

void Color::setG(int _g) 
{   
    this->g = Utils::unitClamp(static_cast<float>(_g) / 255.0f);
}

void Color::setB(int _b) 
{
    this->b = Utils::unitClamp(static_cast<float>(_b) / 255.0f);
}

bool operator==(const Color& c1, const Color& c2)
{
    return (c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b);
}

bool operator!=(const Color& c1, const Color& c2)
{
    return (c1.r != c2.r) && (c1.g != c2.g) && (c1.b != c2.b);
}

const Color operator+(const Color& c1, const Color& c2)
{
    return Color(c1.r + c2.r, c1.g + c2.g, c1.b + c2.b);
}

const Color operator*(const Color& c1, const Color& c2)
{
    return Color(c1.r * c2.r, c1.g * c2.g, c1.b * c2.b);
}

const Color operator*(const Color& c, const float scale)
{
    return Color(c.r * scale, c.g * scale, c.b * scale);
}

const Color operator*(const float scale, const Color& c)
{
    return Color(c.r * scale, c.g * scale, c.b * scale);
}

const Color operator/(const Color& c, const float scale)
{
    return Color(c.r / scale, c.g / scale, c.b / scale);
}

Color& Color::operator+=(const Color &c)
{
    this->r = Utils::unitClamp(this->r + c.r);
    this->b = Utils::unitClamp(this->b + c.b);
    this->g = Utils::unitClamp(this->g + c.g);
    return *this;
}

Color& Color::operator-=(const Color &c)
{
    this->r = Utils::unitClamp(this->r - c.r);
    this->b = Utils::unitClamp(this->b - c.b);
    this->g = Utils::unitClamp(this->g - c.g);
    return *this;
}

Color& Color::operator*=(float scale)
{
    this->r = Utils::unitClamp(this->r * scale);
    this->b = Utils::unitClamp(this->b * scale);
    this->g = Utils::unitClamp(this->g * scale);
    return *this;
}

Color& Color::operator*=(int scale)
{
    this->r = Utils::unitClamp(this->r * scale);
    this->b = Utils::unitClamp(this->b * scale);
    this->g = Utils::unitClamp(this->g * scale);
    return *this;
}

Color& Color::operator/=(float scale)
{
    this->r = Utils::unitClamp(this->r / scale);
    this->b = Utils::unitClamp(this->b / scale);
    this->g = Utils::unitClamp(this->g / scale);
    return *this;
}

/**
 * Returns the luminosity of the color in the range [0,1]
 */
float Color::luminosity() const
{
	return (0.21f * this->fR()) + (0.72f * this->fG()) + (0.07f * this->fB());
}

/**
 * Courtesy of "Color Conversion Algorithms"
 * http://www.cs.rit.edu/~ncs/color/t_convert.html
 */
void Color::toHSV(float* h, float* s, float* v) const
{
    float rV    = this->fR(); 
    float gV    = this->fG(); 
    float bV    = this->fB(); 
    float minV  = min(r, min(g, b));
    float maxV  = max(r, max(g, b));
    float delta = maxV - minV;

    *v = maxV;

    if (maxV != 0.0f) {
        *s = delta / maxV;
    } else {
        // r = g = b = 0; s = 0, v is undefined
        *s = 0.0f;
        *h = -1.0f;
        return;
    }

    if (rV == maxV) {
        *h =  (gV - bV) / delta; // between yellow & magenta
    } else if (gV == maxV) {
        *h = 2.0f + (bV - rV) / delta; // between cyan & yellow
    } else {
        *h = 4.0f + (rV - gV) / delta; // between magenta & cyan
    }

    *h *= 60.0f; // degrees
    
    if (*h < 0.0f) {
        *h += 360.0f;
    }
}

std::ostream& operator<<(std::ostream& s, const Color& color)
{
    return s << "Color(" << color.fR() << "," 
                         << color.fG() << "," 
                         << color.fB() << ")";
}

/*****************************************************************************/
