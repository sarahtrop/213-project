#if !defined(CREATURE_HH)
#define CREATURE_HH

#include <cmath>
#include "vec2d.hh"
#include "header.hh"

class creature {
public:
  // Create a new star with a given position and velocity
  creature(vec2d pos, vec2d vel, int color[8]) : 
      _pos(pos),
      _vel(vel),
      _color(color) {}
  /*
  // Update this star's position with a given force and a change in time
  void update(double dt) {
    vec2d accel = _force / _mass;
    
    // Verlet integration
    if(!_initialized) { // First step: no previous position
      vec2d next_pos = _pos + _vel * dt + accel / 2 * dt * dt;
      _prev_pos = _pos;
      _pos = next_pos;
    } else {  // Later steps: 
      vec2d next_pos = _pos * 2 - _prev_pos + accel * dt * dt;
      _prev_pos = _pos;
      _pos = next_pos;
    }
    
    // Track velocity, even though this isn't strictly required
    _vel += accel * dt;
    
    } */
  
  // Get the position of this creature
  vec2d pos() { return _pos; }
  
  // Get the velocity of this creature
  vec2d vel() { return _vel; }
  
  // Get the color of this creature
  rgb32 color() {
    int intColor = binaryToInt(color);
    return rgb32(intColor, intColor, intColor);
  }

  // Get the food source of this creature
  int food_source() { return _food_source; }
  
  // Get the radius of this creature
  double radius() { return _binaryToInt(size); } // TODO: write binaryToDec

  /*
  // Merge two stars
  star merge(star other) {
    double mass = _mass + other._mass;
    vec2d pos = (_pos * _mass + other._pos * other._mass) / (_mass + other._mass);
    vec2d vel = (_vel * _mass + other._vel * other._mass) / (_mass + other._mass);
    
    rgb32 color = rgb32(
      ((double)_color.red*_mass + (double)other._color.red*other._mass) / (_mass + other._mass),
      ((double)_color.green*_mass + (double)other._color.green*other._mass) / (_mass + other._mass),
      ((double)_color.blue*_mass + (double)other._color.blue*other._mass) / (_mass + other._mass));
    
    return star(mass, pos, vel, color);
  }
  */
  
private:
  double _mass;       // The mass of this creature
  vec2d _pos;         // The position of this creature
  vec2d _prev_pos;    // The previous position of this creature
  vec2d _vel;         // The velocity of this creature
  
  int food_source;    // Herbivore (0) or carnivore (1)
  int color[8];       // Color of the creature
  int size[8];        // Size of the creature
  int speed[8];       // Speed of the creature
  int vision[8];      // Distance the creature can see
  int energy[8];      // Max energy of the creature
  int metabolism[8];  // Metabolism of the creature
};

#endif
