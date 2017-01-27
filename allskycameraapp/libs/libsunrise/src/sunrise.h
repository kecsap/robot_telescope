/**
 * Sunrise
 * Copyright (C) 2015 Simon Cooksey, Jonathan Poole, and the University of Kent
 */

#include <math.h>
#include <time.h>

#ifndef __sunrise_h_
#define __sunrise_h_

typedef struct colour_t {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} colour_t;

class Sunrise {
public:
    Sunrise() :
                timezone_c(0),
                A_SD(-0.0144862328)
                {
                    set_lat_long(51.2750, -1.0870);
                };

    /**
     * Get the current latitude
     */
    double getLatitude() { return latitude_c; }


    /**
     * Get the current Longitude
     */
    double getLongitude() { return longitude_c; }

    /**
     * Set the location for which we will calculate the sunrise and sunset.
     *
     * +ve latitude: Northern Hemisphere, -ve Latitude: Southern Hemisphere
     * +ve Longitude: West of Greenwich, -ve Longitude: East of Greenwich
     */
    void set_lat_long(double latitude, double longitude);


    /**
     * Calculate the time of sunrise
     *
     * Returns: a time_t seconds at which the next sunrise will be.
     */
    time_t get_sunrise();

    /**
     * Calculate the time of sunset
     *
     * Returns: a time_t seconds at which the next sunset will be.
     */
    time_t get_sunset();

    void print_time(time_t seconds);
private:
    double longitude_c, latitude_c, timezone_c;

    // Altitude of the solar disc
    const double A_SD;

    double get_fractional_time(time_t time);
    double get_equation_of_time(double fractional_time);
    time_t get_solar_noon(double fractional_time);
    double get_sun_declination(double fractional_time);
    double get_sr_hour_angle(double fractional_time);
};

#endif // __sunrise_h_
