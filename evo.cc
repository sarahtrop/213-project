#include <cstdio>
#include <ctype.h>
#include <vector>
#include <pthread.h>
#include <thread>

#include "creature.hh"
#include "gui.hh"
#include "threads.hh"

using namespace std;

#define NUM_CREATURES 1

// Initialize creatures in the simulation
void initCreatures();

// Update all creatures in the simulation
void updateCreatures();

// Draw a circle on a bitmap based on this creature's position and radius
void drawCreature(bitmap* bmp, creature c);
void drawPlant(bitmap* bmp, plant p);

// Find the nearest food source and change velocity vector
void findNearestFood(creature * c);

// List of creatures
vector<creature> creatures;
vector<plant> plants;

taskQueue_t * q;

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
  initTaskQueue(q);

  while(running) {    
    // Update creature positions
    updateCreatures();

    // Darken the bitmap instead of clearing it to leave trails
    bmp.darken(0.60);

    double prob = rand() % 10;
    if(prob <= 2){
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
          bmp->set(center_x + x, center_y + y, c.color());
          bmp->set(center_x + x, center_y - y, c.color());
          bmp->set(center_x - x, center_y - y, c.color());
          bmp->set(center_x - x, center_y + y, c.color());
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
    creatures[i].update();
    creatures[i].decEnergy();
    if(creatures[i].curr_energy() <= 0){
      creatures.erase(creatures.begin() + i);
      --i;
    }
   findNearestFood(&creatures[i]);
  }
  
  //This checks for collisions
  for(int i=0; i<creatures.size(); ++i){
    //Check for creature collisions
    for(int j=i+1; j < creatures.size(); ++j){
      creatures[i].checkCreatureCollision(&creatures[j]);
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

void initCreatures() {
  for (int i = 0; i < NUM_CREATURES; i++) {
    creature new_creature = creature(0, 128, 128, 128, 128, 128);
    creatures.push_back(new_creature);
  }
}


// Finds the nearest food to a creature, and
// changes the creatures velocity to go towards that creature.
void findNearestFood(creature * c) {
  // If a carnivore, not needed
  if (c->food_source() == 1) {
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

  if (closest == NULL) {
    return;
  }
  // Now that we have the closest plant,
  // change the creature's velocity vector to go towards that plant
  vec2d cPos = c->pos();
  vec2d pPos = closest->pos();
  vec2d towards = vec2d(pPos.x() - cPos.x(), pPos.y() - cPos.y());
  //printf("New Vec: <%f, %f>\n", towards.x(), towards.y());
  c->setVel(towards);
}
