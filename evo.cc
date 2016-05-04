//gui.hh taken from galaxy lab written by Charlie Curtsinger

#include <cstdio>
#include <stdint.h>
#include <ctype.h>
#include <vector>
#include <pthread.h>
#include <thread>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <cmath>

#include "creature.hh"
#include "gui.hh"

using namespace std;

#define NUM_CREATURES 5

// Update all creatures in the simulation
void updateCreatures();

// Draw a circle on a bitmap based on this creature's position and radius
void drawCreature(bitmap* bmp, creature * c);
// Draw a random plant for eating
void drawPlant(bitmap* bmp, plant * p);

// Initialize creatures in the simulation
void initCreatures();

// Perform the functions needed on each creature each frame
void handleTick(int i);

// Find the nearest food source and change velocity vector
void findNearestFood(creature * c);

// Find the nearest herbivore to eat and change velocity vectors
void findNearestHerbivore(creature* c);
// If you are an herbivore, run away from carnivores near you
void runAway(creature* c);

// Find a buddy for reproduction
void findNearestBuddy(creature* c);
// Reproduce
void reproduce(creature* c, creature* d);
// Create new trait for reproduction
uint8_t new_trait(creature* c, creature* d, int trait);
// check if the creatures are similar enough to reproduce
bool reproductionSimilarity(creature* c, creature* d);
unsigned GetTickCount();

// List of creatures
vector<creature*> creatures;
vector<plant*> plants;

// Screen size
#define WIDTH 960
#define HEIGHT 720

int main(int argc, char** argv) {
  // Seed the random number generator
  srand(time(NULL));
  
  // Create a GUI window
  gui ui("Evolution Simulation", WIDTH, HEIGHT);
  
  // Start with the running flag set to true
  bool running = true;
  
  // Render everything using this bitmap
  bitmap bmp(WIDTH, HEIGHT);

  initCreatures();
  //initPlants();
  initTaskQueue();

  unsigned int next_tick;
  while(running) {
    next_tick = GetTickCount();

    // Update creature positions
    updateCreatures();

    // Darken the bitmap instead of clearing it to leave trails
    bmp.darken(0.60);

    double prob = rand() % 10;
    if(prob <= 9){
      plant * newPlant = new plant();
      plants.push_back(newPlant);
    }
    
    //Draw plants
    for (int i = 0; i < plants.size(); ++i){
      drawPlant(&bmp, plants[i]);
    }

    // Draw creatures
    for (int i = 0; i < creatures.size(); i++) {
      drawCreature(&bmp, creatures[i]);
      creatures[i]->setStatus(3);
    }
	
    // Display the rendered frame
    ui.display(bmp);
    unsigned int cur_time = GetTickCount();
    unsigned int diff = cur_time - next_tick;
    
    if(diff < 1000/FPS){
      usleep((1000/FPS - diff) * 1000);
    }
  }
  
  return 0;
}


// Draw a circle at the given creature's position
// Uses method from http://groups.csail.mit.edu/graphics/classes/6.837/F98/Lecture6/circle.html
void drawCreature(bitmap* bmp, creature * c) {

  double center_x = c->pos().x();
  double center_y = c->pos().y();
  double radius = c->radius();
  rgb32 border_color;
  rgb32 inner_color = c->color();

  if (c->status() == 1) { //If reproducing, turn pink
    inner_color = rgb32(219, 112, 147);
  }

  // Checking creature's food source to determine border color
  if (c->food_source() == 1) {
    border_color = rgb32(255, 0, 0);
  }
  else {
    border_color = rgb32(0, 255, 0);
  }
  
  // Loop over points in the upper-right quad of the circle
  for(double x = 0; x <= radius*1.1; x++) {
    for(double y = 0; y <= radius*1.1; y++) {
      // Is this point within the circle's radius?
      double dist = sqrt(pow(x, 2) + pow(y, 2));
      if(dist < radius) {
        if (dist > radius - 3) {
          bmp->set(center_x + x, center_y + y, border_color);
          bmp->set(center_x + x, center_y - y, border_color);
          bmp->set(center_x - x, center_y - y, border_color);
          bmp->set(center_x - x, center_y + y, border_color);
        }
        else {
          // Set this point, along with the mirrored points in the other three quads
          bmp->set(center_x + x, center_y + y, inner_color);
          bmp->set(center_x + x, center_y - y, inner_color);
          bmp->set(center_x - x, center_y - y, inner_color);
          bmp->set(center_x - x, center_y + y, inner_color);
        }
      }
    }
  }
}

void drawPlant(bitmap* bmp, plant * p){
  double center_x = p->pos().x();
  double center_y = p->pos().y();
  double radius = p->radius();
  rgb32 color = rgb32(64, 64, 255);
  
  // Loop over points in the upper-right quad of the circle
  for(double x = 0; x <= radius*1.1; x++) {
    for(double y = 0; y <= radius*1.1; y++) {
      // Is this point within the circle's radius?
      double dist = sqrt(pow(x, 2) + pow(y, 2));
      if(dist < radius) {
        // Set this point, along with the mirrored points in the other three quads
        bmp->set(center_x + x, center_y + y, color);
        bmp->set(center_x + x, center_y - y, color);
        bmp->set(center_x - x, center_y - y, color);
        bmp->set(center_x - x, center_y + y, color);
      }
    }
  }
}

// Compute force on all creatures and update their positions
void updateCreatures(){
  //This updates position and checks for energy level
  for(int i=0; i<creatures.size(); ++i) {
    addTask(&handleTick, i);
  }
  
  //This checks for collisions
  for(int i=0; i<creatures.size(); ++i) {

    //Check for creature collisions
    for(int j=i+1; j < creatures.size(); ++j) {
      bool * collideStatus = creatures[i]->checkCreatureCollision(creatures[j]);
      if (collideStatus[0]) { // If trying to reproduce
        reproduce(creatures[i], creatures[j]);
      }
      if (collideStatus[1]) { // If trying to eat
        creatures[i]->incEnergy((creatures[j]->curr_energy())/10);
        creatures.erase(creatures.begin() + j);
      }
    }

    //Check for plant collisions
    if(creatures[i]->food_source() == 0){
      for(int j = 0; j < plants.size(); ++j){
        bool collided = plants[j]->checkCreatureCollision(creatures[i]);
        if(collided){
          plants.erase(plants.begin() + j);
          --j;
          creatures[i]->incEnergy();
        }
      }
    }
  }
}

// Initialize creatures
void initCreatures() {
  for (int i = 0; i < NUM_CREATURES; i++) {
    creature * new_creature = new creature(0, 128, 128, 128, 128, 128);
    creatures.push_back(new_creature);
  }
  for (int i = 0; i < NUM_CREATURES / 5; ++i){
    creature * new_creature = new creature(1, 128, 128, 128, 128, 128);
    creatures.push_back(new_creature);
  }
}

// Perform the functions needed on each creature each frame
void handleTick(int i) {
  pthread_mutex_lock(&creatures[i]->lock); // lock this creature
  
  if(creatures[i]->curr_energy() <= 0){ // if the creature has no energy
    pthread_mutex_unlock(&creatures[i]->lock); // unlock
    creatures.erase(creatures.begin() + i); // die
    --i;
  }
  else{
    if(!creatures[i]->bouncing()){ // if the creature is not bouncing off another
      runAway(creatures[i]); // either run away
      findNearestBuddy(creatures[i]); // or find a buddy
      findNearestFood(creatures[i]); // or find food
    }
    else{
      creatures[i]->setBouncing(false);
    }
    
    creatures[i]->update(); // update the creatures position and such
    creatures[i]->decEnergy(); // decrement the energy of the creature
  }
  pthread_mutex_unlock(&creatures[i]->lock); // unlock the creature
}

// Finds the nearest food to a creature, and change velocity vector
void findNearestFood(creature * c) {
  // If a carnivore, go find an herbivore
  if (c->food_source() == 1) {
    findNearestHerbivore(c);
    return;
  }
  else if (c->status() < 2) { // choose to reproduce or run away over eat
    return;
  }

  // Set the minimum distance as the farthest it can be to be visible
  double minDist = c->vision();
  // Se the current closest plant to the first one
  plant* closest = (plant *)malloc(sizeof(plant));
  // Find the closest plant, and save it
  for (int i = 0; i < plants.size(); i++) {
    double curr_dist = plants[i]->distFromCreature(*c);
    if (curr_dist < minDist) {
      minDist = curr_dist;
      closest = plants[i];
    }
  }

  // If the distance is still vision, we found nothing, so don't reset the vector
  if (minDist != c->vision()) {
    // Now that we have the closest plant,
    // change the creature's velocity vector to go towards that plant
    vec2d cPos = c->pos();
    vec2d pPos = closest->pos();
    vec2d towards = vec2d(pPos.x() - cPos.x(), pPos.y() - cPos.y());
    c->setVel(towards);
    c->setStatus(2);
  }
}

// Find the nearest herbivore to eat and change velocity vectors
void findNearestHerbivore(creature* c) {
  // If reproducing or an herbivore, keep doing your thing
  if (c->status() == 1 || c->food_source() == 0) { 
    return;
  }
  
  // Set the minimum distance as the farthest it can be to be visible
  double minDist = c->vision();
  // Se the current closest creature to the first one
  creature* closest = (creature *)malloc(sizeof(creature));
  // Find the closest creature, and save it
  for (int i = 0; i < creatures.size(); i++) {
    // Make sure we are eating an herbivore
    if (creatures[i]->food_source() == 0) {
      double curr_dist = creatures[i]->distFromCreature(*c);
      if(curr_dist < minDist){
        minDist = curr_dist;
        closest = creatures[i];
      }
    }
  }

  // If the distance is still vision, we found nothing, so don't reset the vector
  if (minDist != c->vision()) { 
    // Now that we have the closest creature for eating,
    // change the creature's velocity vector to go towards that creature
    vec2d cPos = c->pos();
    vec2d ePos = closest->pos();
    vec2d towards = ePos - cPos;
    c->setVel(towards);
    c->setStatus(2);
  }
}

void runAway(creature* c) {
  // If you are a carnivore, you never have to run away
  if (c->food_source() == 1) { 
    return;
  }

  double minDist = c->vision();
  bool found = false;
  vec2d away = vec2d(0,0);
    
  // Find the closest creature, and save it
  for (int i = 0; i < creatures.size(); i++) {
    creature* carnivore = creatures[i];
    // Make sure we are running from a carnivore
    if (carnivore->food_source() == 1) {
      double curr_dist = carnivore->distFromCreature(*c);
      if(curr_dist <= minDist){
        away = (away + (c->pos() - carnivore->pos()).normalized()).normalized();
        found = true;
      }
    }
  }

  // If the creature is not us, RUN AWAY
  if (minDist != c->vision()) { 
    c->setVel(away);
    c->setStatus(0); //Set status to RUN AWAY
  }
}


// Finds the nearest buddy for reproduction
void findNearestBuddy(creature* c) {
  if (c->status() < 1) { // If we are being chased, DON'T FIND A BUDDY
    return;
  }
  
  if ((c->curr_energy() / c->max_energy()) >= 0.7) {
    double minDist = c->vision();
    creature* closest = (creature *)malloc(sizeof(creature));
    int type = c->food_source();
    
    // Find the closest creature, and save it
    for (int i = 0; i < creatures.size(); i++) {
      creature* buddy = creatures[i];
      double curr_dist = buddy->distFromCreature(*c);
      if (curr_dist != 0) { // Make sure our buddy is not us
        // Check qualifications for reproduction
        if (curr_dist < minDist && //Did we find a buddy
            buddy->food_source() == type && //Is the buddy our food type
            buddy->curr_energy() / buddy->max_energy() >= 0.7 && //Does buddy have the energy
            reproductionSimilarity(c, buddy) && //Are we the same species
            buddy->status() > 1) { //Is buddy not someone else's buddy
          minDist = curr_dist;
          closest = buddy;
        }
      }
    }

    // go towards buddy
    if (closest->status() > 1 && minDist != c->vision()) { 
      vec2d cPos = c->pos();
      vec2d bPos = closest->pos();
      vec2d towardsB = vec2d(bPos.x() - cPos.x(), bPos.y() - cPos.y());
      vec2d towardsC = vec2d(cPos.x() - bPos.x(), cPos.y() - bPos.y());
      c->setVel(towardsB);
      closest->setVel(towardsC);
      c->setStatus(1);
      closest->setStatus(1);
    }
  }
}

// Reproduce with new creature
void reproduce(creature* c, creature* d) {

  // Set the status of the parents back to doing nothing
  c->setStatus(3);
  d->setStatus(3);
  
  // Create new baby creature
  creature * baby = new creature(c->food_source(), new_trait(c, d, 0),
                                 new_trait(c, d, 1), new_trait(c, d, 2),
                                 new_trait(c, d, 3), new_trait(c, d, 4));

  // Add baby creature to the vector
  creatures.push_back(baby);

  // Deplete parents energy
  c->halfEnergy();
  d->halfEnergy();
}

// Create new trait from that of the parents
uint8_t new_trait(creature* c, creature* d, int trait) {

  uint8_t parent1 = c->getTrait(trait);
  uint8_t parent2 = d->getTrait(trait);

  int mut = rand() % 4;
  int mutBit = -1;
  if(mut == 0){ //25% chance of mutation
    mutBit = rand() % 8; //Selects the bit for mutation
  }
  
  uint8_t ret; //The new value for the creature
  for (int i = 0; i < 8;  i++) { //For each bit in the trait
    int parent = rand() % 2; // Select a random parent
    if (parent == 0) { //Take trait from parent1
      //If the bit in the ith position is a 1, add a 1 in that position
      if((uint8_t)(pow(2,i)) & parent1){ 
        ret += (uint8_t)(pow(2,i));
      }
    }
    else{ //take trait from parent 2
      //If the bit in the ith position is a 1, add a 1 in that position
      if((uint8_t)(pow(2,i)) & parent2){
        ret += (uint8_t)(pow(2,i));
      }
    }
  }

  if(mutBit != -1){ // If we are mutating, mutate
    if(ret & (uint8_t)(pow(2,mutBit))){
      ret -= (uint8_t)(pow(2,mutBit));
    }
    else{
      ret += (uint8_t)(pow(2,mutBit));
    }
  }
  
  return ret;
}

// Check if the creatures are similar enough to reproduce
bool reproductionSimilarity(creature* c, creature* d) {
  int count = 0;
  
  for (int i = 0; i < 5; i++) { // iterate over all 5 traits
    uint8_t p1 = c->getTrait(i);
    uint8_t p2 = d->getTrait(i);
    for (int j = 0; j < 8; j++) { // iterate over bits in trait
      if (((uint8_t)(2^i) & p1) != ((uint8_t)(2^i) & p2)) {
        ++count;
      }
    }
  }
  
  if (count < 8) {
    return true;
  }
  else {
    return false;
  }
}

//The similar function as Window's function GetTickCount 
//code from: http://www.doctort.org/adam/nerd-notes/linux-equivalent-of-the-windows-gettickcount-function.html
unsigned GetTickCount()
{
  struct timeval tv;
  if(gettimeofday(&tv, NULL) != 0)
    return 0;

  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
