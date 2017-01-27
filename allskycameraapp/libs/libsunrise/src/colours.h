/**
 * Sunrise
 * Copyright (C) 2015 Simon Cooksey, Jonathan Poole, and the University of Kent
 */

#ifndef __colours_h_
#define __colours_h_

#include "sunrise.h"

class Colours {
public:
    Colours(Sunrise srise) : sr(srise) {}

    /**
     * Convert the current time into an RGB colour to display.
     */
    colour_t get_colour(time_t time);
private:
    Sunrise sr;

    colour_t temperature_to_colour(int temperature);
};

#endif // __colours_h_
