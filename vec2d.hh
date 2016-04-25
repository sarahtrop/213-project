#ifndef VEC2D_HH
#define VEC2D_HH

#include <cmath>

#include "bitmap.hh"

class vec2d {
public:
  // Create a vector
  vec2d(double x, double y) : _x(x), _y(y) {}
  
  vec2d() : _x(0), _y(0) {}
  
  // Getters for x, y, and z
  double x() { return _x; }
  double y() { return _y; }
  
  // Add another vector to this one and return the result
  vec2d operator+(const vec2d& other) {
    return vec2d(_x + other._x, _y + other._y);
  }
  
  // Add another vector to this one and update in place
  vec2d& operator+=(const vec2d& other) {
    _x += other._x;
    _y += other._y;
    return *this;
  }
  
  // Negate this vector
  vec2d operator-() {
    return vec2d(-_x, -_y);
  }
  
  // Subtract another vector from this one and return the result
  vec2d operator-(const vec2d& other) {
    return vec2d(_x-other._x, _y-other._y);
  }
  
  // Subtract another vector from this one and update in place
  vec2d& operator-=(const vec2d& other) {
    _x -= other._x;
    _y -= other._y;
    return *this;
  }
  
  // Multiply this vector by a scalar and return the result
  vec2d operator*(double scalar) {
    return vec2d(_x*scalar, _y*scalar);
  }
  
  // Multiply this vector by a scalar and update in place
  vec2d& operator*=(double scalar) {
    _x *= scalar;
    _y *= scalar;
    return *this;
  }
  
  // Divide this vector by a scalar and return the result
  vec2d operator/(double scalar) {
    return vec2d(_x/scalar, _y/scalar);
  }
  
  // Divide this vector by a scalar and update in place
  vec2d& operator/=(double scalar) {
    _x /= scalar;
    _y /= scalar;
    return *this;
  }
  
  // Compute the dot product of this vector with another vector
  double operator*(const vec2d& other) {
    return _x*other._x + _y*other._y;
  }
  
  // Compute the magnitude of this vector
  double magnitude() {
    return sqrt(pow(_x, 2) + pow(_y, 2));
  }
  
  // Compute a normalized version of this vector
  vec2d normalized() {
    return (*this) / this->magnitude();
  }
  
private:
  double _x;
  double _y;
};

#endif
