// An exploding firework effect for a horizontal strip
//
// History:   2023-02-04    rrbauer   Created
//            2023-02-12    rrbauer   Added fractional drawing (thanks davepl!), random particle fade
//            2023-02-13    rrbauer   Added size element to Particle struct

/** Notes
Pick a random spot for the firework to explode, light it up, flicker, fade
Explode into a number of bright particles that move outward at various speeds, gradually decreasing in speed
Fade them slowly at first, then more quickly as they burn out (not implemented yet)
**/

#include <Arduino.h>
#include <FastLED.h>
#include "FractionalDrawing.h"

void launch(unsigned int location);
void explode(unsigned int location);

#define DATA_PIN    6
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    64

CRGBArray<NUM_LEDS> leds;

void setup() {
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS);
  leds.fill_solid(CRGB::Black);
  // Serial.begin(9600);
}

void loop() {
  unsigned int location = random(NUM_LEDS);
  launch(location);     // initial spot, flicker?, fade
  explode(location);    // explosion
  FastLED.delay(1000);  // wait a bit before doing it again
}


void launch(unsigned int location) {
  // light it up
  leds[location] = CRGB::DarkOrange;
  FastLED.delay(100);

  // fade it out
  // iterations developed visually and using fadeToBlack spreadsheet
  for (int i=0; i<21; i++) {
    leds[location].fadeToBlackBy(50);
    FastLED.delay(10);
  }
}

struct Particle {
  int size;
  float location;
  float velocity;
  uint8_t hue;
  uint8_t value;
  int fadeby;
  bool active;
};

// explode into a random number of particles, each with
// initial size, velocities, fading and slowing with time
// I started with standard motion equations, but these didn't work
// for a 1-d horizonal strip; they always favored "falling" (moving toward
// one end of the strip), not spreading
void explode(unsigned int location) {
  int num_particles = random8(20, 32);
  Particle particles[num_particles];
  int active_particles = num_particles;
  // float acceleration = -0.5;
  uint8_t hue = random8();

  // initialize particles array
  for (int i=0; i<num_particles; i++) {
    particles[i].size = random(1, 5);
    particles[i].location = location;
    particles[i].velocity = random(-500, 500) / 100.0;
    particles[i].hue = hue;
    particles[i].value = 255;
    particles[i].fadeby = random(5, 15);
    particles[i].active = true;
  }

  while (active_particles > 0) {
    // display the particles
    FastLED.clear();
    for (int i=0; i<num_particles; i++) {
      if (particles[i].active) {
        if (particles[i].value > 0 && particles[i].location >= 0 && particles[i].location < NUM_LEDS){
          // leds[particles[i].location] = CHSV(particles[i].hue, 255, particles[i].value);
          // DrawPixels(particles[i].location, 1, CHSV(particles[i].hue, 255, particles[i].value));
          DrawPixels(particles[i].location, particles[i].size, CHSV(particles[i].hue, 255, particles[i].value));
        } else {
          particles[i].active = false;
          active_particles--;
        }
      }
    }
    // delay, which also displays
    FastLED.delay(50);
    
    // move & fade the particles
    // these are the basic motion equations i started with
    // turns out they don't work for simulating horizontal motion without modification
    // leaving them here for future reference...
    // s = s0 + v0t + ½at2 (2nd equation of motion)
    // v = v0 + at (1st equation of motion)
    // t is always 1 here
    for (int i=0; i<num_particles; i++) {
      // if velocity is negative, flip sign of acceleration
      // float accel_v = (particles[i].velocity > 0) ? acceleration : -acceleration;
      // particles[i].location += particles[i].velocity + accel_v/2;
      // particles[i].velocity += accel_v;
      particles[i].location += particles[i].velocity;
      particles[i].velocity *= 0.9;
      // this logic is to prevent underflow, value is uint8_t, which would just wrap around
      particles[i].value = (particles[i].value >= particles[i].fadeby) ? particles[i].value - particles[i].fadeby : 0;
    }
  }
}
