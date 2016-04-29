#if !defined(CREATURE_HH)
#define CREATURE_HH

#define MAX_RADIUS 20
#define MIN_RADIUS 6
#define MAX_ENERGY 28
#define MIN_ENERGY 4

#define FPS 50
// Screen size
#define WIDTH 960
#define HEIGHT 720

#include <cmath>
#include <ctime>
#include "vec2d.hh"
#include "header.hh"

class creature {
public:
  
  creature(int food_source, uint8_t color, uint8_t size, uint8_t speed, uint8_t energy, uint8_t vision) : 
    _food_source(food_source),
    _size(size),
    _speed(speed),
    _energy(energy),
    _vision(vision),
    _color(color),
    _collided(false) {
      setPos();
      setVel();
      _curr_energy((int)energy / 2);
  }

    creature(int food_source, uint8_t color, uint8_t size, uint8_t speed, uint8_t energy, uint8_t vision, vec2d pos, vec2d vel) : 
    _food_source(food_source),
    _size(size),
    _speed(speed),
    _energy(energy),
    _vision(vision),
    _color(color),
    _collided(false) {
      setPos(pos);
      setVel(vel);
      _curr_energy((int)energy / 2);
  }
  
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
    return rgb32((int)_color, (int)_color, (int)_color);
  }

  // Get the food source of this creature
  int food_source() { return _food_source; }
  
  // Get the radius of this creature
  double radius() { return (_size / 255.0) * (MAX_RADIUS - MIN_RADIUS) + MIN_RADIUS; }

  double speed() { return (_speed / FPS); }

  vec2d getPos(){ return _pos; }

  vec2d getVel(){ return _vel; }

  bool getCollided(){ return _collided; }

  void setCollided(bool coll){
    _collided = coll;
  }

  //Randomly sets the position of the creature within passed bounds
  void setPos(){
    _pos = vec2d(rand() % (WIDTH - (int)ceil(2*radius())) + radius(), rand() % (HEIGHT - (int)ceil(2*radius())) + radius());
  }

  //Sets the position to the given position
  void setPos(vec2d pos){
    _pos = pos; 
  }

  //Sets the velocity vector to a randomized normal vector
  void setVel(){
    double dir = rand() * 2 * 3.141;
    double x = cos(dir);
    double y = sin(dir);
    _vel = vec2d(x,y).normalized();
  }

  //Sets the velocity vector to the normalized passed vector
  void setVel(vec2d vel){
    _vel = vel.normalized();
  }

  // Increments current energy when food is eaten
  void incEnergy() {
    _curr_energy += ((int)_energy / 9) - 4;

    // If the energy level is higher or equal to the max possible, set to max
    if (_curr_energy >= (int)_energy)
      _curr_energy = (int)_energy;
  }

  // Decrements energy as time passes
  void decEnergy(){
    _curr_energy -= ((int)_energy / 9) - 4;

    // Ensuring energy is never 0
    if (_curr_energy <= 0)
      _curr_energy = 0;
  }

  void update(){
    if(_pos.y()-radius() < 0 && _vel.y() < 0){
      setVel(vec2d(_vel.x(), -1*_vel.y()));
    }
    if(_pos.x()-radius() < 0 && _vel.x() < 0){
      setVel(vec2d(-1*_vel.x(), _vel.y()));
    }
    if(_pos.y()+radius() > HEIGHT && _vel.y() > 0){
      setVel(vec2d(_vel.x(), -1*_vel.y()));
    }
    if(_pos.x()+radius() > WIDTH && _vel.x() > 0){
      setVel(vec2d(-1*_vel.x(), _vel.y()));
    }
    
    _pos += (_vel * speed());
  }

  void checkCollision(creature * partner){
    vec2d partPos = (*partner).getPos();
    vec2d partVel = (*partner).getVel();
    
    double dist = sqrt(pow((_pos.x() - partPos.x()), 2) + pow((_pos.y() - partPos.y()), 2));
    //If a collision has occured
    if(dist <= radius() + (*partner).radius() && intersects(partner)){
      vec2d partPos = (*partner).getPos();
      vec2d partVel = (*partner).getVel();

      //https://nicoschertler.wordpress.com/2013/10/07/elastic-collision-of-circles-and-spheres/  
      vec2d normal = vec2d(_pos.x() - partPos.x(), _pos.y() - partPos.y()).normalized();

      //Get dot products
      double c1dot = normal * _vel;
      double c2dot = normal * partVel;

      double p = c1dot - c2dot;
      
      setVel(_vel - normal * p);
      (*partner).setVel(partVel + normal * p);
    }
  }

  bool intersects(creature * partner){
    vec2d partPos = (*partner).getPos();
    vec2d partVel = (*partner).getVel();
    double u = (_pos.y()*partVel.x() + partVel.y()*partPos.x() - partPos.y()*partVel.x() - partVel.y()*_pos.x()) / (_vel.x()*partVel.y() - _vel.y()*partVel.x());

    double v = (_pos.x() + _vel.x() * u - partPos.x()) / partVel.x();
    
    if(u > 0 || v > 0){
      return true;
    }
    return false;
  }

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
  bool _collided;
  double _mass;       // The mass of this creature
  vec2d _pos;         // The position of this creature
  vec2d _prev_pos;    // The previous position of this creature
  vec2d _vel;         // The velocity of this creature

  double _act_size;
  double _curr_energy;
  
  int _food_source;    // Herbivore (0) or carnivore (1)
  uint8_t _color;       // Color of the creature
  uint8_t _size;        // Size of the creature
  uint8_t _speed;       // Speed of the creature
  uint8_t _vision;      // Distance the creature can see
  uint8_t _energy;      // Max energy of the creature
  int _metabolism;  // Metabolism of the creature
};

#endif
