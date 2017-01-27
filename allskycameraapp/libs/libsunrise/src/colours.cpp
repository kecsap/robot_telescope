#include <math.h>

#include "colours.h"

colour_t Colours::temperature_to_colour(int temperature)
{
    int r, g, b;
    if(temperature <= 66) {
        r = 255;
    }
    else {
        r = temperature - 60;
        r = (int) (329.698727446 * pow((float)r, -0.1332047592));
        if(r < 0) r = 0;
        if(r > 255) r = 255;
    }

    if(temperature <= 66) {
        g = temperature;
        g = 99.4708025861 * log(g) - 161.1195681661;
        if(g < 0) g = 0;
        if(g > 255) g = 255;
    } else {
        g = temperature - 60;
        g = 288.1221695283 * pow(g, -0.0755148492);
        if(g < 0) g = 0;
        if(g > 255) g = 255;
    }

    if (temperature >= 66) {
        b = 255;
    } else {
        b = temperature - 10;
        b = 138.5177312231 * log(b) - 305.0447927307;
        if(b < 0) b = 0;
        if(b > 255) b = 255;
    }

    colour_t colour;
    colour.red = r;
    colour.blue = b;
    colour.green = g;
    return colour;
}

colour_t Colours::get_colour(time_t time)
{
    struct tm tm = *localtime(&time);
    colour_t colour = temperature_to_colour(66);
    float luminosity = 1.0f;

    // Calculate the time to the next sunrise/sunset
    int time_of_day = (tm.tm_hour-1) * 60 * 60 + tm.tm_min * 60 + tm.tm_sec;
    int time_of_sunset = sr.get_sunset();
    int time_of_sunrise = sr.get_sunrise();

    // Sunrise from 1000k to 6600k
    if(time_of_day - time_of_sunrise < 60 * 60 && time_of_day - time_of_sunrise > 0)
    {
        colour = temperature_to_colour(5 + ((time_of_day - time_of_sunrise)/60));
        // printf("Sunrise. %dK\n", 10+18*(time_of_day - time_of_sunrise)/60);
    }

    // Sunset from 6600k to 1000k
    if(time_of_sunset - time_of_day < 60 * 60 && time_of_sunset - time_of_day > 0)
    {
        colour = temperature_to_colour(5 + ((time_of_sunset - time_of_day)/60));
        // printf("Sunset. %dK\n", 10+18*(time_of_day - time_of_sunrise)/60);
    }

    if(time_of_day <= time_of_sunrise)
    {
        colour = temperature_to_colour(5);
        // Sunrise is less than an hour away
        if(time_of_sunrise - time_of_day < 30) {
            luminosity = 0.033*((time_of_sunrise - time_of_day)/30);
        }
        else
            luminosity = 0.033;
    }

    if(time_of_day >= time_of_sunset)
    {
        colour = temperature_to_colour(5);
        if(time_of_sunset - time_of_day < 30) {
            luminosity = 1.0 - 0.033*((time_of_day - time_of_sunset)/30);
        }
        else
            luminosity = 0.033;
    }

    colour.red *= luminosity;
    colour.green *= luminosity;
    colour.blue *= luminosity;

    return colour;
}
