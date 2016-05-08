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
#include <fstream>

#include "creature.hh"
#include "gui.hh"

using namespace std;

#define NUM_CREATURES 40

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
void writeData();
void generatePlants();


// List of creatures
vector<creature*> creatures;
vector<plant*> plants;

int frames = 0;

double thisTime;

const char* fName = "data8.txt";

int main(int argc, char** argv) {
  // Seed the random number generator
  srand(time(NULL));
  
  // Create a GUI window
  gui ui("Evolution Simulation", WIDTH, HEIGHT);
  
  // Start with the running flag set to true
  bool running = true;
  
  // Render everything using this bitmap
  bitmap bmp(WIDTH, HEIGHT);

  ofstream file;
  file.open(fName, ios::trunc); //Clear File
  file << "Plant Generation,Plants,Herbivores,Carnivores,ProcessSpeed,Size,Speed,Energy,Vision\n";
  file.close();

  initCreatures();
  initTaskQueue();

  unsigned int next_tick;
  while(running) {
    next_tick = GetTickCount();

    // Update creature positions
    updateCreatures();

    // Darken the bitmap instead of clearing it to leave trails
    bmp.darken(0.60);

    generatePlants();
    
    //Draw plants
    for (int i = 0; i < plants.size(); ++i){
      drawPlant(&bmp, plants[i]);
    }

    // Draw creatures
    for (int i = 0; i < creatures.size(); i++) {
      drawCreature(&bmp, creatures[i]);
      creatures[i]->setStatus(3);
    }

    if(frames % 10 == 0){
      writeData();
      printf("%d\n", frames);
    }
	
    // Display the rendered frame
    ui.display(bmp);
    ++frames;
    
    unsigned int cur_time = GetTickCount();
    unsigned int diff = cur_time - next_tick;
    
    if(diff < 1000/FPS){
      usleep((1000/FPS - diff) * 1000);
    }
  }
  
  return 0;
}

void generatePlants(){
  double rawPlants = 1.25*cos(2*3.1415*frames/10000)+1.75;

  int f1 = rawPlants * 1000;
  int f2 = (rawPlants - 1) * 1000;
  int f3 = (rawPlants - 2) * 1000;

  double prob = rand() % 1000;
  
  if(f1 >= prob){
    plant * newPlant = new plant();
    plants.push_back(newPlant);
  }
  if(f2 >= prob){
    plant * newPlant = new plant();
    plants.push_back(newPlant);
  }
  if(f3 >= prob){
    plant * newPlant = new plant();
    plants.push_back(newPlant);
  }
}

void writeData(){
  int carn = 0;
  int herb = 0;

  int size = 0;
  int speed = 0;
  int energy = 0;
  int vision = 0;

  std::fstream file;
  file.open(fName, ios::app); 
  
  for(int i = 0; i < creatures.size(); ++i){
    creature * c = creatures[i];
    size += c->getTrait(1);
    speed += c->getTrait(2);
    energy += c->getTrait(3);
    vision += c->getTrait(4);
    
    if(c->food_source() == 0){
      ++herb;
    }
    else{
      ++carn;
    }
  }

  size = (double)size / creatures.size();
  speed = (double)speed / creatures.size();
  energy = (double)energy / creatures.size();
  vision = (double)vision / creatures.size();

  file << cos(2*3.1415*frames/10000)+2;
  file << ",";
  file << plants.size();
  file << ",";
  file << herb;
  file << ",";
  file << carn;
  file << ",";
  file << thisTime;
  file << ",";
  file << size;
  file << ",";
  file << speed;
  file << ",";
  file << energy;
  file << ",";
  file << vision;
  file << "\n";

  file.close();

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
  resetTasks();

  struct timeval tv;
  gettimeofday(&tv, NULL);

  double cur_time = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);//GetTickCount();
    
  //This updates position and checks for energy level
  for(int i=0; i<creatures.size(); ++i) {
    addTask(&handleTick, i);
  }

  pthread_mutex_lock(&countTasks);
  while(tasksFinished < creatures.size()){
    pthread_cond_wait(&countCond, &countTasks);
  }
  pthread_mutex_unlock(&countTasks);

  
  gettimeofday(&tv, NULL);

  double end_time = (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);//GetTickCount();

  thisTime = end_time - cur_time;
  
  //This checks for collisions
  for(int i=0; i<creatures.size(); ++i) {

    //Check for creature collisions
    for(int j=i+1; j < creatures.size(); ++j) {
      bool * colStatus = creatures[i]->checkCreatureCollision(creatures[j]);
      if (colStatus[0]) { // If trying to reproduce
        reproduce(creatures[i], creatures[j]);
      }

      if(colStatus[1]){
        if(creatures[i]->food_source() == 1){
          creatures[i]->incEnergy((creatures[j]->curr_energy()));
          creatures[j]->incEnergy(-10000);
        }
        else{
          creatures[j]->incEnergy((creatures[i]->curr_energy()));
          creatures[i]->incEnergy(-10000);
        }
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

    if(creatures[i]->curr_energy() <= 0){ // if the creature has no energy
      creatures.erase(creatures.begin() + i); // die
      --i;
      }
  }
}

// Initialize creatures
void initCreatures() {
  for (int i = 0; i < NUM_CREATURES; i++) {
    creature * new_creature = new creature(0, 128, 128, 128, 128, 128);
    creatures.push_back(new_creature);
  }
  for (int i = 0; i < NUM_CREATURES / 10; ++i){
    creature * new_creature = new creature(1, 128, 128, 128, 128, 128);
    creatures.push_back(new_creature);
  }
  
}

// Perform the functions needed on each creature each frame
void handleTick(int i) {
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
    if (creatures[i]->food_source() == 0 && c->canEat(creatures[i])) {
      double curr_dist = c->distFromCreature(*creatures[i]) - creatures[i]->radius();
      if(curr_dist < minDist){
        minDist = curr_dist;
        closest = creatures[i];
      }
    }
  }

  // If the distance is still vision, we found nothing, so don't reset the vector
  if (minDist < c->vision()) {
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
    if (carnivore->food_source() == 1 && carnivore->canEat(c)) {
      double curr_dist = c->distFromCreature(*carnivore) - creatures[i]->radius();
      if(curr_dist <= minDist){
        away = (away + (c->pos() - carnivore->pos()).normalized()).normalized();
        found = true;
      }
    }
  }

  // If the creature is not us, RUN AWAY
  if (found) { 
    c->setVel(away);
    c->setStatus(0); //Set status to RUN AWAY
  }
}


// Finds the nearest buddy for reproduction
void findNearestBuddy(creature* c) {
  if (c->status() < 1) { // If we are being chased, DON'T FIND A BUDDY
    return;
  }

  double matingDist = c->vision() * 2;
  
  if ((c->curr_energy() / c->max_energy()) >= 0.7) {
    double minDist = matingDist;
    creature* closest = (creature *)malloc(sizeof(creature));
    int type = c->food_source();
    
    // Find the closest creature, and save it
    for (int i = 0; i < creatures.size(); i++) {
      creature* buddy = creatures[i];
      double curr_dist = buddy->distFromCreature(*c);
      if (curr_dist != 0) { // Make sure our buddy is not us
        // Check qualifications for reproduction
        if (curr_dist < minDist && //Did we find a closer buddy
            buddy->food_source() == type && //Is the buddy our food type
            (buddy->curr_energy() / buddy->max_energy()) >= 0.7 && //Does buddy have the energy
            reproductionSimilarity(c, buddy)){ //Are we the same species
          minDist = curr_dist;
          closest = buddy;
        }
      }
    }

    // go towards buddy
    if (closest->status() > 0 && minDist < matingDist) { 
      vec2d cPos = c->pos();
      vec2d bPos = closest->pos();
      vec2d towardsB = vec2d(bPos.x() - cPos.x(), bPos.y() - cPos.y());
      //vec2d towardsC = vec2d(cPos.x() - bPos.x(), cPos.y() - bPos.y());
      c->setVel(towardsB);
      //closest->setVel(towardsC);
      c->setStatus(1);
      //closest->setStatus(1);
    }
  }
}

// Reproduce with new creature
void reproduce(creature* c, creature* d) {

  // Set the status of the parents back to doing nothing
  c->setStatus(3);
  d->setStatus(3);

  int carnMut = rand() % 100;
  int children = 1;
  int food = c->food_source();

  if(carnMut <= 1){
    children = 4;
    food = 1;
  }

  for(int i = 0; i < children; ++i){
    // Create new baby creature
    creature * baby = new creature(food, new_trait(c, d, 0),
                                   new_trait(c, d, 1), new_trait(c, d, 2),
                                   new_trait(c, d, 3), new_trait(c, d, 4));

    //baby->print();

    // Add baby creature to the vector
    creatures.push_back(baby);
  }

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
  
  uint8_t ret = 0; //The new value for the creature
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
  
  for (int i = 1; i < 5; i++) { // iterate over all 4 traits
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

  return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}
