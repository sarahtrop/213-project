#include <sdl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "creature.hh"
#include "gui.hh"
#include "header.hh"

using namespace std;

// Screen size
#define WIDTH 640
#define HEIGHT 480

// Blocks and threads
#define THREADS_PER_BLOCKS 2
#define BLOCKS 4

// List of creatures
vector<creature> creatures;

// Update all creatures in the simulation
void updateCreatures();

// Draw a circle on a bitmap based on this creature's position and radius
void drawCreature(bitmap* bmp, creature c);

int main(int argc, char** argv) {
  // Seed the random number generator
  srand(time(NULL));
  
  // Create a GUI window
  gui ui("Evolution Simulation", WIDTH, HEIGHT);
  
  // Start with the running flag set to true
  bool running = true;
  
  // Render everything using this bitmap
  bitmap bmp(WIDTH, HEIGHT);

  while(running) {
    // Update creature positions
    updateCreatures();

    // Darken the bitmap instead of clearing it to leave trails
    bmp.darken(0.92);

    // Draw creatures
    drawCreature(&bmp, creatures[i]);
    
    // Display the rendered frame
    ui.display(bmp);
  }
  
  return 0;
}

// Draw a circle at the given creature's position
// Uses method from http://groups.csail.mit.edu/graphics/classes/6.837/F98/Lecture6/circle.html
void drawCreature(bitmap* bmp, creature c) {
  // Index of the creature? Have a struct of creatures? how do we make this GPU compatible
  // int index = blockIdx.x * THREADS_PER_BLOCK + threadIdx.x;

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
        if (dist > radius - 5) {
          bmp->set(center_x + x + x_offset, center_y + y + y_offset, border_color);
          bmp->set(center_x + x + x_offset, center_y - y + y_offset, border_color);
          bmp->set(center_x - x + x_offset, center_y - y + y_offset, border_color);
          bmp->set(center_x - x + x_offset, center_y + y + y_offset, border_color);
        }
        else {
          // Set this point, along with the mirrored points in the other three quads
          bmp->set(center_x + x + x_offset, center_y + y + y_offset, c.color());
          bmp->set(center_x + x + x_offset, center_y - y + y_offset, c.color());
          bmp->set(center_x - x + x_offset, center_y - y + y_offset, c.color());
          bmp->set(center_x - x + x_offset, center_y + y + y_offset, c.color());
        }
      }
    }
  }
}
