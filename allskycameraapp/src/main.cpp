/**
 *  This file is part of allskycameraapp
 *
 *  Copyright (C) 2017 Csaba Kertész (csaba.kertesz@gmail.com)
 *
 *  AiBO+ is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  AiBO+ is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include <core/MANum.hpp>

#include <MEHistogram.hpp>
#include <MEImage.hpp>

#include <MCLog.hpp>

#include <sunrise.h>

#include <QDateTime>
#include <QProcess>
#include <QTime>

#include <geoclue.h>

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

int GetGmtOffset()
{
  time_t UtcTime = time(NULL);
  struct tm LocalTime = { 0 };
  int Offset = 0;

  localtime_r(&UtcTime, &LocalTime);
  Offset = LocalTime.tm_gmtoff / 60 / 60;
  MC_LOG("GMT offset is %d hours.", Offset);
  MC_LOG("The time zone is '%s'.", LocalTime.tm_zone);
  return Offset;
}


void GetLocation(float& longitude, float& latitude)
{
  GError* Error = NULL;
  GClueSimple* GeoClueContext = gclue_simple_new_sync("firefox", GCLUE_ACCURACY_LEVEL_EXACT, NULL, &Error);
  GClueLocation* GeoLocation = gclue_simple_get_location(GeoClueContext);

  if (Error != NULL)
  {
    MC_WARNING("GeoClue2 error: %s", Error->message);
  }
  longitude = gclue_location_get_longitude(GeoLocation);
  latitude = gclue_location_get_latitude(GeoLocation);
}


tm GetTime(time_t time)
{
    struct tm Time = *localtime(&time);

    return Time;
}


float GetSunArea(MEImage& image)
{
    MEHistogram Histogram;
    int PixelCount = image.GetImageDataSize();
    float SunArea = 0;

    Histogram.Calculate(image, MEHistogram::h_Overwrite);
    for (int i = 240; i < 256; ++i)
    {
      SunArea += Histogram.HistogramData[i];
    }
    SunArea /= (float)PixelCount;
    return SunArea;
}


int main(int argc, char * argv[])
{
  printf("allskycameraapp\n");
  printf("Developed by Csaba Kertesz (csaba.kertesz@gmail.com)\n\n");

  if (argc != 3)
  {
    printf("Usage: allskycameraapp Wunderground_camera_ID Wunderground_password\n");
    return 1;
  }

  MCLog::SetCustomHandler(new MCLogEcho);
  MCLog::SetDebugStatus(true);

  const int GmtCorrection = GetGmtOffset();
  float Longitude = 0;
  float Latitude = 0;
  Sunrise SunCalc;
  tm SunriseTime;
  tm SunsetTime;
  int NightMode = -1;
  MANum<int> ShutterTime(50, 10, 6000000);
  MANum<int> Iso(800, 100, 800);
  MEImage MaskImage;
  MEImage InfoLayerImage;
  bool InfoLayer = false;
  bool LongWait = false;

  GetLocation(Longitude, Latitude);
  MC_LOG("Current location - longitude: %1.4f latitude: %1.4f", Longitude, Latitude);
  SunCalc.set_lat_long(Latitude, -Longitude);
  SunriseTime = GetTime(SunCalc.get_sunrise());
  SunsetTime = GetTime(SunCalc.get_sunset());
  MC_LOG("Sunrise: %02d:%02d", SunriseTime.tm_hour, SunriseTime.tm_min);
  MC_LOG("Sunset: %02d:%02d", SunsetTime.tm_hour+GmtCorrection, SunsetTime.tm_min);
  if (!MCFileExists("mask.png"))
  {
    MC_WARNING("Mask image (mask.png) does not exist!");
    MaskImage.Realloc(640, 384, 3);
    MaskImage.DrawRectangle(0, 0, 639, 383, MEColor(255, 255, 255), true);
  } else {
    MC_LOG("Loading mask image...");
    MaskImage.LoadFromFile("mask.png");
    MaskImage.Invert();
    MaskImage.ConvertToRGB();
  }
  if (MCFileExists("info_layer.jpg"))
  {
    MC_LOG("Loading info layer image...");
    InfoLayerImage.LoadFromFile("info_layer.jpg");
    InfoLayer = true;
  }

  while (true)
  {
    QTime CurrentTime = QTime::currentTime();
    QTime Sunrise, Sunset;

    // Check the current time and select daytime or night shutter mode based on sunset/sunrise
    SunriseTime = GetTime(SunCalc.get_sunrise());
    Sunrise = QTime(SunriseTime.tm_hour, SunriseTime.tm_min);
    SunsetTime = GetTime(SunCalc.get_sunset());
    Sunset = QTime(SunsetTime.tm_hour+GmtCorrection, SunsetTime.tm_min);
    if ((NightMode == -1 || NightMode == 0) && (CurrentTime > Sunset || CurrentTime < Sunrise) &&
        // No night mode in too sunny summer locations
        SunsetTime.tm_hour+GmtCorrection <= 23)
    {
      MC_LOG("Change to night mode");
      ShutterTime = 4500000;
      Iso = 800;
      NightMode = 1;
    } else
    if ((NightMode == -1 || NightMode == 1) &&
        ((CurrentTime > Sunrise && CurrentTime < Sunset) || SunsetTime.tm_hour+GmtCorrection > 23))
    {
      MC_LOG("Change to daylight mode");
      ShutterTime = (NightMode == -1 ? 500 : 4500000);
      Iso = 100;
      NightMode = 0;
    } else
    if (NightMode == 0 && Iso == 100 && CurrentTime > QTime(Sunset.hour()-1, Sunset.minute()) && CurrentTime < Sunset)
    {
      MC_LOG("Change to daylight mode (high ISO mode)");
      Iso = 800;
    }
    if (NightMode == 0)
    {
      printf("Wait %d\n", LongWait);
      sleep(LongWait ? 1200 : 30);
      LongWait = false;
    }
    // Capture an image
    QString CommandStr;

    CommandStr = QString("raspistill -ISO %1 -sa %2 -n -w 640 -h 384 -ss %3 -vf -hf -o /tmp/capture.png").arg((int)Iso).
                 arg(NightMode == 0 ? 20 : 0).arg((int)ShutterTime);
    printf("Start capture\n");
    QProcess::execute(CommandStr);
    if (!MCFileExists("/tmp/capture.png"))
    {
      MC_WARNING("Image was not captured!");
      sleep(30);
      continue;
    }
    // Change the shutter speed if needed
    MEImage CapturedImage, TempImage;
    ME::ImageSPtr BlueLayer;
    QString Text;

    CapturedImage.LoadFromFile("/tmp/capture.png");
    CapturedImage.SaveToFile((QString("/home/pi/pics/allskycam_%1_%2.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMdd")).
                              arg(QDateTime::currentDateTime().toString("HHmm"))).toStdString());

    TempImage = CapturedImage;
    TempImage.Mask(MaskImage);
    TempImage.GammaCorrection(0.3);
    TempImage.Threshold(140);
    BlueLayer.reset(TempImage.GetLayer(2));
    TempImage = *BlueLayer;
    TempImage.ConvertToRGB();
    // Shutter time control
    int Brightness = 0;
    float SunArea = 0;

    if (NightMode == 0)
    {
      SunArea = GetSunArea(CapturedImage);
      Brightness = (int)CapturedImage.AverageBrightnessLevel();
      if (Brightness > 180 || SunArea > 0.007)
      {
        LongWait = true;
        ShutterTime = 10;
//        if (SunArea >= 0.1)
//          ShutterTime = (int)((float)ShutterTime / 4);
//        else
//          ShutterTime = (int)((float)ShutterTime / 2);
        MC_LOG("Average brightness: %d - Sun area: %1.4f - Decrease shutter time to %d", Brightness, SunArea, (int)ShutterTime);
      } else
      if (Brightness < 100 && SunArea < 0.02)
      {
        ShutterTime = (int)((float)ShutterTime*1.2);
        MC_LOG("Average brightness: %d - Increase shutter time to %d", Brightness, (int)ShutterTime);
      }
      // Do gamma correction if the shutter speed is very short because of direct sunlight
      if (Brightness < 100)
        CapturedImage.GammaCorrection(0.5);
    } else {
      Brightness = (int)CapturedImage.AverageBrightnessLevel();

//      MC_LOG("Average brightness: %d", Brightness);
      if (Brightness > 200)
      {
        ShutterTime = (int)((float)ShutterTime / 1.3);
        MC_LOG("Average brightness: %d - Decrease shutter time to %d", Brightness, (int)ShutterTime);
      } else
      if (Brightness <= 10)
      {
        ShutterTime = 4500000;
        MC_LOG("Average brightness: %d - Reset shutter time to %d", Brightness, (int)ShutterTime);
      }
    }
    // Clear sky detection and info layer composition in night mode
    if (NightMode == 1)
    {
      CapturedImage.GammaCorrection(0.5);
      if ((float)TempImage.GetWhitePixelCount() / CapturedImage.GetHeight() / CapturedImage.GetWidth() / 3 > 5)
      {
        Text = QString("Clouds or Moon");
      } else {
        Text = QString("Clear Sky");
        if (InfoLayer)
          CapturedImage.Addition(InfoLayerImage, ME::NonNegativeSumAddition);
      }
      CapturedImage.DrawText(CapturedImage.GetWidth()-220, CapturedImage.GetHeight()-25, Text.toStdString(),
                             0.8, MEColor(255, 255, 255));
    }
    // Save the final image
    CapturedImage.SaveToFile("/tmp/capture.jpg");
    // Upload the image to Wunderground
    QString UploadCommandStr;

    UploadCommandStr = QString("curl -s -S -T /tmp/capture.jpg ftp://webcam.wunderground.com --user %1:%2").arg(argv[1]).arg(argv[2]);
    QProcess::execute(UploadCommandStr);
    MC_LOG("Image uploaded (brightness: %d, sunarea: %1.4f)", Brightness, SunArea);
    // Wait some time
    sleep(40);
  }
  return 0;
}
