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

#include "inference.h"

#include <core/MANum.hpp>

#include <MEHistogram.hpp>
#include <MEImage.hpp>

#include <MCLog.hpp>

#include <SmtpMime>
#include <sunrise.h>

#include <QCommandLineParser>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTime>

#include <opencv2/core.hpp>

#include <geoclue.h>

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

int GetGmtOffset()
{
  time_t UtcTime = time(nullptr);
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
  GError* Error = nullptr;
  GClueSimple* GeoClueContext = gclue_simple_new_sync("firefox", GCLUE_ACCURACY_LEVEL_EXACT, nullptr, &Error);
  GClueLocation* GeoLocation = gclue_simple_get_location(GeoClueContext);

  if (Error != nullptr)
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


bool ValidateImage(MEImage& image, const QString& filename = "")
{
  return true;

  if (image.GetLayerCount() != 3 || image.GetWidth() != 160 || image.GetHeight() != 96)
  {
    printf("Warning: invalid image properties %dx%dx%d instead of 160x96x3\n", image.GetWidth(),
           image.GetHeight(), image.GetLayerCount());
    return false;
  }
  // OpenCV stores the channels in reverse order
  std::unique_ptr<MEImage> LayerR(image.GetLayer(2));
  std::unique_ptr<MEImage> LayerG(image.GetLayer(1));
  std::unique_ptr<MEImage> LayerB(image.GetLayer(0));

  if (LayerR->AverageBrightnessLevel() / 12 > LayerG->AverageBrightnessLevel())
  {
    printf("Invalid image - %s (%1.2f, %1.2f, %1.2f)\n", qPrintable(filename), LayerR->AverageBrightnessLevel(),
           LayerG->AverageBrightnessLevel(), LayerB->AverageBrightnessLevel());
    return false;
  }
  return true;
}


void SendEmailNotification(const QString& smtp_user, const QString& smtp_pass, const QString& recipient_email)
{
  SmtpClient Smtp("smtp.gmail.com", 465, SmtpClient::SslConnection);

  Smtp.setUser(smtp_user);
  Smtp.setPassword(smtp_pass);

  MimeMessage Message;
  MimeText Text;

  // Notification email header
  Message.setSender(new EmailAddress(smtp_user, "Skycam"));
  Message.addRecipient(new EmailAddress(recipient_email, ""));
  Message.setSubject("Clear Sky Alert");
  // Notification email text
  Text.setText("Hi,\n\nCheck the sky, it seems to be clear!\n\nRegards,\nSkycam Janitor");
  Message.addPart(&Text);

  // Send the e-mail
  if (!Smtp.connectToHost())
  {
    printf("Unable to connect to the SMTP server\n");
  } else {
    if (!Smtp.login())
    {
      printf("Unable to login to the SMTP server\n");
    } else {
      if (!Smtp.sendMail(Message))
      {
        printf("Unable to send e-mail notification to %s\n", qPrintable(recipient_email));
      }
    }
  }
  Smtp.quit();
}


int main(int argc, char* argv[])
{
  QCoreApplication App(argc, argv);
  std::unique_ptr<CppInference> SkyModel;

  printf("allskycameraapp\n");
  printf("Developed by Csaba Kertesz (csaba.kertesz@gmail.com)\n\n");

  // Parse command line arguments
  QCommandLineParser Parser;
  QCommandLineOption CameraIDOption({"c", "cameraid"}, "Wunderground camera ID", "cameraid");
  QCommandLineOption PasswordOption({"p", "password"}, "Wunderground password", "password");
  QCommandLineOption PathOption({"i", "imagepath"}, "Path to save images", "imagepath");
  QCommandLineOption WebFileOption({"w", "webfile"}, "Static web image", "webfile");
  QCommandLineOption ModelOption({"m", "modelprefix"}, "Model prefix name (model.{data-00000-of-00001,index,meta})", "modelprefix");
  QCommandLineOption MaskOption({"M", "maskimage"}, "Mask image", "maskimage", "mask.png");
  QCommandLineOption TestImageOption({"t", "testimage"}, "Test image or directory", "testimage");
  QCommandLineOption SortOption({"s", "sort"}, "Sort images in image path", "sort");
  QCommandLineOption SmtpUserOption({"U", "smtpuser"}, "GMail SMTP username", "smtpuser");
  QCommandLineOption SmtpPassOption({"P", "smtppass"}, "GMail SMTP password", "smtppass");
  QCommandLineOption EmailOption({"e", "email"}, "Notification e-mail address", "email");

  Parser.addHelpOption();
  Parser.addOption(CameraIDOption);
  Parser.addOption(PasswordOption);
  Parser.addOption(PathOption);
  Parser.addOption(WebFileOption);
  Parser.addOption(ModelOption);
  Parser.addOption(MaskOption);
  Parser.addOption(TestImageOption);
  Parser.addOption(SortOption);
  Parser.addOption(SmtpUserOption);
  Parser.addOption(SmtpPassOption);
  Parser.addOption(EmailOption);
  Parser.process(App);


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
  if (!MCFileExists(Parser.value(MaskOption).toStdString()))
  {
    MC_WARNING("Mask image (%s) does not exist!", qPrintable(Parser.value(MaskOption)));
    MaskImage.Realloc(640, 384, 3);
    MaskImage.DrawRectangle(0, 0, 639, 383, MEColor(255, 255, 255), true);
  } else {
    MC_LOG("Loading mask image...");
    MaskImage.LoadFromFile(Parser.value(MaskOption).toStdString());
    MaskImage.Invert();
    MaskImage.ConvertToRGB();
  }
  if (MCFileExists("info_layer.jpg"))
  {
    MC_LOG("Loading info layer image...");
    InfoLayerImage.LoadFromFile("info_layer.jpg");
    InfoLayer = true;
  }

  if (Parser.isSet("modelprefix"))
  {
    SkyModel.reset(new CppInference());
    SkyModel->Load(Parser.value(ModelOption));
    if (Parser.isSet("testimage") &&
        (QFile(Parser.value(TestImageOption)).exists() || QDir(Parser.value(TestImageOption)).exists()))
    {
      QStringList Files;
      int FilesCount = 0;
      bool PathMode = true;

      if (QDir(Parser.value(TestImageOption)).exists())
      {
        QStringList Filenames = QDir(Parser.value(TestImageOption)).entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg",
                                                                              QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

        for (auto filename : Filenames)
          Files += Parser.value(TestImageOption)+'/'+filename;
      } else {
        PathMode = false;
        Files << Parser.value(TestImageOption);
      }
      int Hits = 0;

      for (auto filename : Files)
      {
        MEImage TestImage;
        std::unique_ptr<MEImage> TempImage;

        TestImage.LoadFromFile(filename.toStdString());
        if (TestImage.GetLayerCount() == 3)
        {
          TempImage.reset(new MEImage(TestImage));
          TempImage->Mask(MaskImage);
          TempImage->Resize(160, 96, true);
          if (!ValidateImage(*TempImage, filename))
          {
            if (PathMode && Parser.isSet("sort"))
            {
              QDir().mkpath(Parser.value(TestImageOption)+"/invalid");
              QFile(filename).rename(Parser.value(TestImageOption)+"/invalid/"+filename.section('/', -1, -1));
            }
            continue;
          }
        }
        if (TestImage.GetLayerCount() == 1)
        {
          TestImage.ConvertToRGB();
        }
        TestImage.Mask(MaskImage);
        TempImage.reset(TestImage.GetLayer(2));
        TempImage->Resize(160, 96, true);
        int Label = SkyModel->Predict(*TempImage);

        if (Label == 0)
        {
          FilesCount++;
          Hits++;
          printf("%s -> Clear\n", qPrintable(filename.section('/', -1, -1)));
          if (PathMode && Parser.isSet("sort"))
          {
            QDir().mkpath(Parser.value(TestImageOption)+"/clear");
            QFile(filename).rename(Parser.value(TestImageOption)+"/clear/"+filename.section('/', -1, -1));
          }
        }
        if (Label == 1)
        {
          FilesCount++;
          printf("%s -> Cloud\n", qPrintable(filename.section('/', -1, -1)));
          if (PathMode && Parser.isSet("sort"))
          {
            QDir().mkpath(Parser.value(TestImageOption)+"/clouds");
            QFile(filename).rename(Parser.value(TestImageOption)+"/clouds/"+filename.section('/', -1, -1));
          }
        }
      }
      if (FilesCount > 0)
      {
        printf("Results: Clear: %1.3f %% - Clouds: %1.3f %% (%d/%d/%d)\n", (float)Hits / FilesCount*100,
               (float)(FilesCount-Hits) / FilesCount*100, Hits, FilesCount-Hits, FilesCount);
      } else {
        printf("No valid images in the requested location: %s\n", qPrintable(Parser.value(TestImageOption)));
      }
      return 0;
    }
  }

  while (true)
  {
    QTime CurrentTime = QTime::currentTime();
    QTime Sunrise, Sunset;
    static int ClearSkyCount = 0;

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
      ClearSkyCount = 0;
    } else
    if ((NightMode == -1 || NightMode == 1) &&
        ((CurrentTime > Sunrise && CurrentTime < Sunset) || SunsetTime.tm_hour+GmtCorrection > 23))
    {
      MC_LOG("Change to daylight mode");
      ShutterTime = (NightMode == -1 ? 500 : 4500000);
      Iso = 100;
      NightMode = 0;
      ClearSkyCount = 0;
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

    CommandStr = QString("raspistill -awb cloud -ISO %1 -sa %2 -n -w 640 -h 384 -ss %3 -vf -hf -o /tmp/capture.png").arg((int)Iso).
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
    MEImage CapturedImage;

    CapturedImage.LoadFromFile("/tmp/capture.png");
    // Clear sky detection with deep learning
    int Clouds = -1;

    if (Parser.isSet("imagepath") && QDir(Parser.value(PathOption)).exists() && NightMode == 1)
    {
      const QString Path = Parser.value(PathOption)+'/';
      const QString FileName = QString("allskycam_%1_%2.jpg").arg(QDateTime::currentDateTime().toString("yyyyMMdd")).
                                arg(QDateTime::currentDateTime().toString("HHmm"));

      if (SkyModel.get())
      {
        std::unique_ptr<MEImage> TempImage(new MEImage(CapturedImage));

        TempImage->Mask(MaskImage);
        TempImage->Resize(160, 96, true);
        if (ValidateImage(*TempImage))
        {
          std::unique_ptr<MEImage> TestImage(CapturedImage.GetLayer(2));

          TestImage->Mask(MaskImage);
          TestImage->Resize(160, 96, true);
          int Label = SkyModel->Predict(*TestImage);

          QDir().mkpath(Path+"clear");
          QDir().mkpath(Path+"clouds");
          if (Label == 0)
          {
            CapturedImage.SaveToFile((Path+"clear/"+FileName).toStdString());
            Clouds = 0;
          }
          if (Label == 1)
          {
            CapturedImage.SaveToFile((Path+"clouds/"+FileName).toStdString());
            Clouds = 1;
          }
        } else {
          QDir().mkpath(Path+"invalid");
          CapturedImage.SaveToFile((Path+"invalid/"+FileName).toStdString());
        }
      } else {
        CapturedImage.SaveToFile((Path+FileName).toStdString());
      }
    }

    // Create an image for upload
    ME::ImageSPtr RedLayer;
    MEImage TempImage;
    QString Text;

    TempImage = CapturedImage;
    TempImage.Mask(MaskImage);
    TempImage.GammaCorrection(0.3);
    TempImage.Threshold(140);
    RedLayer.reset(TempImage.GetLayer(2));
    TempImage = *RedLayer;
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
      if (Clouds == 1 || (Clouds == -1 && (float)TempImage.GetWhitePixelCount() / CapturedImage.GetHeight() / CapturedImage.GetWidth() / 3 > 5))
      {
        Text = QString("Clouds");
      } else {
        ClearSkyCount++;
        if (ClearSkyCount == 20 && Parser.isSet("smtpuser") && Parser.isSet("smtppass") && Parser.isSet("email"))
        {
          SendEmailNotification(Parser.value(SmtpUserOption), Parser.value(SmtpPassOption), Parser.value(EmailOption));
        }
        Text = QString("Clear Sky");
        if (InfoLayer)
          CapturedImage.Addition(InfoLayerImage, ME::NonNegativeSumAddition);
      }
      CapturedImage.DrawText(CapturedImage.GetWidth()-220, CapturedImage.GetHeight()-25, Text.toStdString(),
                             0.8, MEColor(255, 255, 255));
    }
    // Save the final image
    if (Parser.isSet("webfile"))
    {
      CapturedImage.SaveToFile(Parser.value(WebFileOption).toStdString());
    }
    CapturedImage.SaveToFile("/tmp/capture.jpg");
    // Upload the image to Wunderground
    if (Parser.isSet("cameraid") && Parser.isSet("password"))
    {
      QString UploadCommandStr;

      UploadCommandStr = QString("curl -s -S -T /tmp/capture.jpg ftp://webcam.wunderground.com --user %1:%2").
                             arg(Parser.value(CameraIDOption), Parser.value(PasswordOption));
      QProcess::execute(UploadCommandStr);
      MC_LOG("Image uploaded (brightness: %d, sunarea: %1.4f)", Brightness, SunArea);
    } else {
      MC_LOG("Image captured (brightness: %d, sunarea: %1.4f)", Brightness, SunArea);
    }
    // Wait some time
    sleep(40);
  }
  return 0;
}
