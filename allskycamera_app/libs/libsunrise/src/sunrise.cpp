/**
 * Sunrise
 * Copyright (C) 2015 Simon Cooksey, Jonathan Poole, and the University of Kent
 *
 * Equations from http://www.esrl.noaa.gov/gmd/grad/solcalc/solareqns.PDF
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define _USE_MATH_DEFINES
#define DEG_TO_RAD(x) (x/57.2958)
#define RAD_TO_DEG(x) (x*57.2958)

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "sunrise.h"

void Sunrise::set_lat_long(double latitude, double longitude)
{
    latitude_c = latitude;
    longitude_c = longitude;
}

double Sunrise::get_sun_declination(double fractional_time) {
    // Solar declination in radians
    double declination = 0.006918 - 0.399912 * cos(fractional_time) + 0.070257 * sin(fractional_time) -
                         0.006758 * cos(2 * fractional_time) + 0.000907 * sin(2 * fractional_time) -
                         0.002592 * cos(3 * fractional_time) + 0.00148 * sin(3 * fractional_time);

    return declination;
}

double Sunrise::get_fractional_time(time_t time) {
    struct tm tm = *localtime(&time);

    // Time in radians
    return (2 * M_PI / 365) * (tm.tm_yday + (tm.tm_hour - 12 + (tm.tm_min / 60)) / 24);
}

double Sunrise::get_equation_of_time(double fractional_time) {
    return 229.18 * (0.000075 + 0.001868*cos(fractional_time) - 0.032077 * sin(fractional_time) -
           0.014615 * cos(2 * fractional_time) - 0.040849 * sin(2 * fractional_time));
}

time_t Sunrise::get_solar_noon(double fractional_time) {
    double eqtime = get_equation_of_time(fractional_time);
    return (720 + (4 * longitude_c) - eqtime) * 60;
}

void Sunrise::print_time(time_t time) {
    struct tm tm = *localtime(&time);

    printf("%02dh %02dm %02ds\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

double Sunrise::get_sr_hour_angle(double fractional_time) {
    // Declination of the sun in radians
    double declination = get_sun_declination(fractional_time);

    // Sunrise hour angle
    double sunrise_ha = acos((cos(DEG_TO_RAD(90.833)) / (cos(DEG_TO_RAD(latitude_c))*cos(declination))) - (tan(DEG_TO_RAD(latitude_c)) * tan(declination)));
    return sunrise_ha;
}

time_t Sunrise::get_sunrise() {
    double fractional_time = get_fractional_time(time(NULL));
    double eqtime = get_equation_of_time(fractional_time);
    double sunrise_ha = RAD_TO_DEG(get_sr_hour_angle(fractional_time));
    int sunrise = 661 + 4*(longitude_c - sunrise_ha) - eqtime;

    return sunrise * 60;
}

time_t Sunrise::get_sunset() {
    double fractional_time = get_fractional_time(time(NULL));
    double eqtime = get_equation_of_time(fractional_time);
    double sunset_ha = RAD_TO_DEG(0-get_sr_hour_angle(fractional_time));
    int sunset = 661 + 4*(longitude_c - sunset_ha) - eqtime;

    return sunset * 60;
}
