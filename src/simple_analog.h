#pragma once

#include "pebble.h"

static const GPathInfo MINUTE_DIAMOND_POINTS = {
  3,
    (GPoint []) {
    { -5, -54 },
    { 0, -44 }, // -64 keeps you in the band
    { 5, -54 }
  }
};

static const GPathInfo SECOND_HAND_POINTS = {
  4,
  (GPoint []) {
    {1,-255},
      {-2,-255},
      {-2,0},
      {1,0}   
  }
  
};
