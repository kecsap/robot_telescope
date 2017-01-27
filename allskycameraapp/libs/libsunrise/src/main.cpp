#include <stdio.h>
#include <time.h>
#include <math.h>

#include "colours.h"
#include "sunrise.h"

int main(int argc, char * argv[])
{
  Sunrise sr;
  Colours cls(sr);

  sr.set_lat_long(61.4978, -23.7610);   // Tampere
  printf("====== %0.5fN, %0.5fW ======\n", sr.getLatitude(), sr.getLongitude());

  printf("UTC Sunrise: "); sr.print_time(sr.get_sunrise());
  printf("UTC Sunset: "); sr.print_time(sr.get_sunset());

  return 0;
}
