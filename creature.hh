//vec2d.hh taken from galaxy lab written by Charlie Curtsinger

#if !defined(CREATURE_HH)
#define CREATURE_HH

#define MAX_RADIUS 20 //Radius of creature with _size of 255
#define MIN_RADIUS 6 //Radius of creature with _size of 0
#define MAX_ENERGY 28 //Seconds creature will live with _energy of 255
#define MIN_ENERGY 4 //Seconds creature will live with _energy of 0

#define FPS 50
// Screen size
#define WIDTH 960
#define HEIGHT 720

#include <cmath>
#include <ctime>
#include <pthread.h>
#include <thread>

#include "vec2d.hh"

// CREATURE CLASS
class creature {
public:
  
  creature(int food_source, uint8_t color, uint8_t size, uint8_t speed, uint8_t energy, uint8_t vision) : 
    _food_source(food_source),
    _size(size),
    _speed(speed),
    _energy(energy),
    _vision(vision),
    _bouncing(false),
    _color(color){
      setPos();
      setVel();
      setMaxEnergy();
      setMetabolism();
      _curr_energy = (double)_max_energy / 2;
      pthread_mutex_init(&lock, NULL);
      _status = 3;
  }

    creature(int food_source, uint8_t color, uint8_t size, uint8_t speed, uint8_t energy, uint8_t vision, vec2d pos, vec2d vel) : 
    _food_source(food_source),
    _size(size),
    _speed(speed),
    _energy(energy),
    _vision(vision),
    _bouncing(false),
    _color(color){
      setPos(pos);
      setVel(vel);
      setMaxEnergy();
      setMetabolism();
      _curr_energy = (double)_max_energy / 2;
      pthread_mutex_init(&lock, NULL);
      _status = 3;
  }
  
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
  double radius() { return ((double)_size / 255.0) * (MAX_RADIUS - MIN_RADIUS) + MIN_RADIUS; }

  // Get the speed of this creature
  double speed() { return ((double)_speed/ (2 * FPS)) * ((1 -(_size / 255)) * 1.5 + .5); }

  // Get the current energy of this creature
  double curr_energy() { return (double)_curr_energy; }

  // Get the maximum energy
  double max_energy() { return (double)_max_energy; }

  // Get the vision of this creature
  double vision() { return (double)_vision + radius(); }

  // Get a trait 
  uint8_t getTrait(int trait) {
    switch(trait) {
      case 0:
        return _color; 
        break;
      case 1:
        return _size;
        break;
      case 2:
        return _speed;
        break;
      case 3:
        return _energy;
        break;
      case 4:
        return _vision;
        break;
      default:
        return (uint8_t)-1;
        break;
    }
  }

  // Get the status
  int status() { return _status; }

  // Set the status
  void setStatus(int stat) {
    _status = stat;
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

  // If a creature is bouncing off another
  bool bouncing(){
    return _bouncing;
  }

  // Set bouncing boolean
  void setBouncing(bool val){
    _bouncing = val;
  }

  //Sets the maximum energy the creature can have
  void setMaxEnergy(){
    _max_energy = (((double)_energy / 255.0) * (MAX_ENERGY - MIN_ENERGY) + MIN_ENERGY) * FPS;
  }

  //Metabolism directly proportional to the trait values 
  void setMetabolism(){
    _metabolism = pow(((double)(_vision + _size + _speed) / (255*3)) * 1.5 + .5, 2);
  }

  // Increments current energy when food is eaten (inversely proportional to _energy)
  void incEnergy() {
    _curr_energy = fmin(_max_energy, _curr_energy + FPS * ((1 - (_energy / 255)) * 1.5 + .5));
  }

  // Increments current energy by specified amount
  void incEnergy(double add) {
    _curr_energy = fmin(_max_energy, _curr_energy + add);
  }

  // Decrements energy as time passes
  void decEnergy() {
    _curr_energy -= _metabolism;

    // Ensuring energy is never 0
    if (_curr_energy <= 0)
      _curr_energy = 0;
  }

// Used when reproducing, cuts curr_energy in half
  void halfEnergy() {
    _curr_energy -= (_max_energy / 2);
  }

  // Calculate the distance from another creature
  double distFromCreature(creature c) {
    if (c.food_source() == _food_source) {
      vec2d cPos = c.pos();
      return sqrt(pow((_pos.x() - cPos.x()), 2) + pow((_pos.y() - cPos.y()), 2));
    } else {
      return 0.0;
    }
  }

  // Update the position of a creature
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

  // Check if creatures are colliding
  bool * checkCreatureCollision(creature * partner){
    vec2d partPos = (*partner).pos();
    vec2d partVel = (*partner).vel();
    bool * colStatus = (bool*)malloc(sizeof(bool)*2);

    colStatus[0] = false; // for reproduction
    colStatus[1] = false; // for carnivores eating herbivores
    
    double dist = sqrt(pow((_pos.x() - partPos.x()), 2) + pow((_pos.y() - partPos.y()), 2));
    //If a collision has occured
    if(dist <= radius() + (*partner).radius() && intersects(partner)){
      _bouncing = true;
      vec2d partPos = (*partner).pos();
      vec2d partVel = (*partner).vel();

      // If colliding because we've found a buddy
      if (_status == 1 && partner->status() == 1) {
        colStatus[0] = true;
      }

      // If colliding because we are trying to eat someone else
      if (_food_source == 1 && partner->food_source() == 0){// && canEat(partner)){
        colStatus[1] = true;
      }

      //https://nicoschertler.wordpress.com/2013/10/07/elastic-collision-of-circles-and-spheres/  
      vec2d normal = vec2d(_pos.x() - partPos.x(), _pos.y() - partPos.y()).normalized();

      //Get dot products
      double c1dot = normal * _vel;
      double c2dot = normal * partVel;

      double p = c1dot - c2dot;
      
      setVel(_vel - normal * p);
      (*partner).setVel(partVel + normal * p);
    }
    return colStatus;
  }

  // Check is one creature can eat another
  bool canEat(creature * partner){
    bool res = false;
    if(_food_source == 1) {
      if(sqrt(pow(((double)_size / 255) - ((double)partner->getTrait(1) / 255), 2)) <= .1) {
        res = true;
      }
    }
    return res;
  }

  // Check if creatures vectors intersect
  bool intersects(creature * partner){
    vec2d partPos = (*partner).pos();
    vec2d partVel = (*partner).vel();
    double u = (_pos.y()*partVel.x() + partVel.y()*partPos.x() - partPos.y()*partVel.x() - partVel.y()*_pos.x()) / (_vel.x()*partVel.y() - _vel.y()*partVel.x());

    double v = (_pos.x() + _vel.x() * u - partPos.x()) / partVel.x();
    
    if(u > 0 || v > 0){
      return true;
    }
    return false;
  }
  
// Creatures fields
private:
  pthread_mutex_t lock;
  
  double _mass;       // The mass of this creature
  vec2d _pos;         // The position of this creature
  vec2d _prev_pos;    // The previous position of this creature
  vec2d _vel;         // The velocity of this creature

  int _status;         // 0: being chased; 1: finding a buddy; 2: finding food; 3: do nothing

  bool _bouncing;

  //Variables dependent on traits
  double _act_size;
  double _curr_energy;
  double _metabolism; // Metabolism of the creature
  double _max_energy; // Max energy of creature in terms of frames

  //Trait variables
  int _food_source;    // Herbivore (0) or carnivore (1)
  uint8_t _color;       // Color of the creature
  uint8_t _size;        // Size of the creature
  uint8_t _speed;       // Speed of the creature
  uint8_t _vision;      // Distance the creature can see
  uint8_t _energy;      // Max energy of the creature

}; // end of creature class

// PLANT CLASS
class plant {
public:
  
  plant() : _radius(2){
    setPos();
    pthread_mutex_init(&lock, NULL);
  }
    
  // Get the position of this plant
  vec2d pos() { return _pos; }
  
  // Get the radius of the plant
  double radius(){ return _radius; }

  // Check the plant is colliding with a creature
  bool checkCreatureCollision(creature * c){
    vec2d cPos = (*c).pos();
    
    double dist = sqrt(pow((_pos.x() - cPos.x()), 2) + pow((_pos.y() - cPos.y()), 2));
    //If a collision has occured
    if(dist <= radius() + (*c).radius()){
      return true;
    }
    return false;
  }

  // Check the distance from a creature
  double distFromCreature(creature c) {
    vec2d cPos = c.pos();
    return sqrt(pow((_pos.x() - cPos.x()), 2) + pow((_pos.y() - cPos.y()), 2));
  }

  // Set the position of hte plant
  void setPos(){
    _pos = vec2d(rand() % (WIDTH - (int)ceil(2*_radius)) + _radius, rand() % (HEIGHT - (int)ceil(2*_radius)) + _radius);
  }
  
  //Plant fields
  private:
    pthread_mutex_t lock;
  
    vec2d _pos;
    double _radius;
}; // end of plant class

#endif
