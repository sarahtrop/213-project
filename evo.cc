//gui.hh taken from galaxy lab written by Charlie Curtsinger

#include <cstdio>
#include <ctype.h>
#include <vector>
#include <pthread.h>
#include <thread>

#include "threads.hh"
#include "creature.hh"
#include "gui.hh"

using namespace std;

#define NUM_CREATURES 2

// Initialize creatures in the simulation
void initCreatures();

// Update all creatures in the simulation
void updateCreatures();

// Draw a circle on a bitmap based on this creature's position and radius
void drawCreature(bitmap* bmp, creature c);
// Draw a random plant for eating
void drawPlant(bitmap* bmp, plant p);

// Find the nearest food source and change velocity vector
void findNearestFood(creature * c);

// Find the nearest herbivore to eat and change velocity vectors
void findNearestHerbivore(creature* c);
// If you are an herbivore, run away from carnivores near you
void runAway(creature* c);

// Find a buddy for reproduction
bool findNearestBuddy(creature* c);
// Reproduce
void reproduce(creature* c, creature* d);
// Create new trait for reproduction
uint8_t new_trait(creature* c, creature* d, char* trait);
// check if the creatures are similar enough to reproduce
bool reproductionSimilarity(creature* c, creature* d);

// Perform the functions needed on each creature each frame
void handleTick(int i);

// List of creatures
vector<creature> creatures;
vector<plant> plants;

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

  while(running) {    
    // Update creature positions
    updateCreatures();

    // Darken the bitmap instead of clearing it to leave trails
    bmp.darken(0.60);

    double prob = rand() % 10;
    if(prob <= 9){
      plant newPlant = plant();
      plants.push_back(newPlant);
    }

    //Draw plants
    for (int i = 0; i < plants.size(); ++i){
      drawPlant(&bmp, plants[i]);
    }

    // Draw creatures
    for (int i = 0; i < creatures.size(); i++) {
      drawCreature(&bmp, creatures[i]);
    }
    
    // Display the rendered frame
    ui.display(bmp);
  }
  
  return 0;
}


// Draw a circle at the given creature's position
// Uses method from http://groups.csail.mit.edu/graphics/classes/6.837/F98/Lecture6/circle.html
void drawCreature(bitmap* bmp, creature c) {

  double center_x = c.pos().x();
  double center_y = c.pos().y();
  double radius = c.radius();
  rgb32 border_color;
  rgb32 inner_color = c.color();

  if (c.status() == 1) { //If reproducing, turn pink
    inner_color = rgb32(219, 112, 147);
  }

  // Checking creature's food source to determine border color
  if (c.food_source() == 1) {
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

void drawPlant(bitmap* bmp, plant p){
  double center_x = p.pos().x();
  double center_y = p.pos().y();
  double radius = p.radius();
  rgb32 color = rgb32(0, 0, 255);
  
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
    handleTick(i);
  }
  
  //This checks for collisions
  for(int i=0; i<creatures.size(); ++i){
    //Check for creature collisions
    for(int j=i+1; j < creatures.size(); ++j){
      bool isReproducing = creatures[i].checkCreatureCollision(&creatures[j]);
      if (isReproducing) {
        reproduce(&creatures[i], &creatures[j]);
      }
    }
    //Check for plant collisions
    if(creatures[i].food_source() == 0){
      for(int j = 0; j < plants.size(); ++j){
        bool collided = plants[j].checkCreatureCollision(&creatures[i]);
        if(collided){
          plants.erase(plants.begin() + j);
          --j;
          creatures[i].incEnergy();
        }
      }
    }
  }
}

// Initialize creatures
void initCreatures() {
  for (int i = 0; i < NUM_CREATURES; i++) {
    creature new_creature = creature(0, 128, 128, 128, 128, 128);
    creatures.push_back(new_creature);
  }
}

// Perform the functions needed on each creature each frame
void handleTick(int i) {
  if(creatures[i].curr_energy() <= 0){
    creatures.erase(creatures.begin() + i);
    --i;
  }
  else{
    creatures[i].update();
    creatures[i].decEnergy();

    findNearestBuddy(&creatures[i]);
    findNearestFood(&creatures[i]);
    findNearestHerbivore(&creatures[i]);
  }
}

// Finds the nearest food to a creature, and change velocity vector
void findNearestFood(creature * c) {
  // If a carnivore, go find an herbivore
  if (c->food_source() == 1) {
    findNearestHerbivore(c);
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
    double curr_dist = plants[i].distFromCreature(*c);
    if (curr_dist < minDist) {
      minDist = curr_dist;
      closest = &plants[i];
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
  
  double minDist = c->vision();
  creature* closest = (creature *)malloc(sizeof(creature));
    
  // Find the closest creature, and save it
  for (int i = 0; i < creatures.size(); i++) {
    creature* to_eat = &creatures[i];
    // Make sure we are eating an herbivore
    if (to_eat->food_source() == 0) {
      double curr_dist = to_eat->distFromCreature(*c);
      minDist = curr_dist;
      closest = to_eat;
    }
  }

  // If the creature is not us, go towards it
  // Also make the herbivore run away from us
  if (minDist != c->vision()) { 
    vec2d cPos = c->pos();
    vec2d ePos = closest->pos();
    vec2d towards = vec2d(ePos.x() - cPos.x(), ePos.y() - cPos.y());
    c->setVel(towards);
    c->setStatus(2);
    closest->setStatus(0);
  }
}

void runAway(creature* c) {
  // If you are a carnivore, you never have to run away
  if (c->food_source() == 1) { 
    return;
  }

  double minDist = c->vision();
  creature* closest = (creature *)malloc(sizeof(creature));
    
  // Find the closest creature, and save it
  for (int i = 0; i < creatures.size(); i++) {
    creature* carnivore = &creatures[i];
    // Make sure we are running from a carnivore
    if (carnivore->food_source() == 1) {
      double curr_dist = carnivore->distFromCreature(*c);
      minDist = curr_dist;
      closest = carnivore;
    }
  }

  // If the creature is not us, RUN AWAY
  if (minDist != c->vision()) { 
    vec2d mePos = c->pos();
    vec2d carnPos = closest->pos();
    vec2d away = vec2d(carnPos.x() - mePos.x(), carnPos.y() - mePos.y());
    c->setVel(away);
    c->setStatus(0);
  }
  else {
    c->setStatus(3);
  }
}


// Finds the nearest buddy for reproduction
bool findNearestBuddy(creature* c) {
  if (c->status() == 0) { // If we are being chased, DON'T FIND A BUDDY: LIVE
    return false;
  }
  
  if ((c->curr_energy() / c->max_energy()) >= 0.7) {
    double minDist = c->vision();
    creature* closest = (creature *)malloc(sizeof(creature));
    int type = c->food_source();
    
    // Find the closest creature, and save it
    for (int i = 0; i < creatures.size(); i++) {
      creature* buddy = &creatures[i];
      double curr_dist = buddy->distFromCreature(*c);
      if (curr_dist != 0) { // Make sure our buddy is not us
        // Check qualifications for reproduction
        if (curr_dist < minDist &&
            buddy->food_source() == type &&
            buddy->curr_energy() / buddy->max_energy() >= 0.7 &&
            reproductionSimilarity(c, buddy)) {
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
      return true;
    }
  }

  return false;
}

// Reproduce with new creature
void reproduce(creature* c, creature* d) {
  // Create new baby creatures
  creature baby = creature(c->food_source(), new_trait(c, d, 0, new_trait(c, d, 1), 
                        new_trait(c, d, 2), new_trait(c, d, 3), new_trait(c, d, 4));

  // Add baby creature to the vector
  creatures.push_back(baby);

  // Deplete parents energy
  c->halfEnergy();
  d->halfEnergy();

  // Set the status of the parents back to doing nothing
  c->setStatus(3);
  d->setStatus(3);
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
    int parent = rand() % 2;
    if (parent == 0) { //Take trait from parent1
      if((uint8_t)(2^i) && parent1){ //If the bit in the ith position is a 1, add a 1 in that position
        ret += (uint8_t)(2^i);
      }
    }
    else{ //take trait from parent 2
      if((uint8_t)(2^i) && parent2){ //If the bit in the ith position is a 1, add a 1 in that position
        ret += (uint8_t)(2^i);
      }
    }
  }

  if(mutBit != -1){ // If we are mutating, mutate
    if(ret && (uint8_t)(2^mutBit) == 1){
      ret -= (uint8_t)(2^mutBit);
    }
    else{
      ret += (uint8_t)(2^mutBit);
    }
  }
  
  return ret;
}

// Check if the creatures are similar enough to reproduce
bool reproductionSimilarity(creature* c, creature* d) {
  bool traits[8] = {true};

  for (int i = 0; i < 5; i++) { // iterate over all 5 traits
    uint8_t p1 = c->getTrait(i);
    uint8_t p2 = d->getTrait(i);
    for (int j = 0; j < 8; j++) { // iterate over bits in trait
      if (((uint8_t)(2^i) && p1) != ((uint8_t)(2^i) && p2) {
        traits[i] = false; // store which traits are the same between both parents (true)
      }
    }
  }

  // 4 out of 8 is enough to reproduce
  int numTraits = 0;
  for (int i = 0; i < 8; i++) {
    if (traits[i]) {
      numTraits++;
    }
  }
  
  if (numTraits >= 4) {
    return true;
  }
  else {
    return false;
  }
}
