/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include <stdlib.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "driver/adc.h"
#include <WiFi.h>

class Clock {
  // clock widget is 5 * Fontxx.Width wide and Fontxx.Height high
  private:
    PAINT_TIME Paint_time;
    // Paint_time.Sec = 56;
  public:
    UWORD X; //left edge
    UWORD Y; //top edge
    Clock(UWORD Xleft, UWORD Ytop) {
      X = Xleft;
      Y = Ytop;    
    };
    void draw(UWORD hour, UWORD minutes) {
      Paint_time.Hour = hour;
      Paint_time.Min = minutes;
      Paint_ClearWindows(X, Y, X + Font20.Width * 5, Y + Font20.Height, WHITE);
      Paint_DrawTimeHHMM(X, Y, &Paint_time, &Font20, WHITE, BLACK);
      DEV_Delay_ms(500);
    };
};

class IconWithText {
  private:
    UWORD Xmax = 296;
    size_t text_width;
  public:
    UWORD X; //left edge
    UWORD Y; //top edge
    const unsigned char *ImageData;
    UWORD ImageDataW;
    UWORD ImageDataH;
    IconWithText(UWORD Xleft, UWORD Ytop, const unsigned char *Image, UWORD ImageW, UWORD ImageH) : ImageData(Image) {
      X = Xleft;
      Y = Ytop;
      ImageDataW = ImageW;
      ImageDataH = ImageH;
    };
    void drawText(const char *text) {
      text_width = strlen(text);
      Paint_ClearWindows(X + ImageDataW + 4, Y, X + ImageDataW + 4 + Font16.Width * text_width, Y + Font16.Height, WHITE);
      Paint_DrawString_EN(X + ImageDataW + 4, Y, text, &Font16, WHITE, BLACK);
      DEV_Delay_ms(500);
    };
    void drawIcon(bool clearWindows=0) {
      if (clearWindows) {
        // TODO not sure why there is shift between drawing and clearing
        Paint_ClearWindows(X, Y - 4, X + ImageDataW, Y + ImageDataH - 14, WHITE);
      }
      Paint_DrawImage(ImageData, Y, Xmax - X - ImageDataW, ImageDataH, ImageDataW);
      DEV_Delay_ms(500);
    };
};

Adafruit_BME280 bme;
class MeasurmentDevice {
  private:
    IconWithText UI;
    char sprintfBuffer [10];
    UBYTE type;
    void readSensor() {   
      switch (type) {
        case 1:
          property = bme.readTemperature();
          sprintf(sprintfBuffer, "%.0foC", property);
          return;
        case 2:
          property = bme.readHumidity();
          sprintf(sprintfBuffer,  "%.0f%%", property);
          return;
        case 3:
          property = bme.readPressure() / 100.0F;
          sprintf(sprintfBuffer, "%.0fhPa", property);
          return;
      }
    }
  public:
    float property;
    MeasurmentDevice(UWORD Xleft, UWORD Ytop, const unsigned char *Image, UWORD ImageW, UWORD ImageH, UBYTE DeviceType) 
      : UI(Xleft, Ytop, Image, ImageW, ImageH) {
      type = DeviceType; 
    };     
    void draw(bool clearWindows=0) {
      UI.drawIcon(clearWindows) ;
      readSensor();
      UI.drawText(sprintfBuffer);    
    };
    void update() {
      readSensor();
      UI.drawText(sprintfBuffer);    
    };
};

#define mS_TO_S_FACTOR 1000
#define uS_TO_S_FACTOR 1000000
UBYTE *ImageCache;
UWORD CPU_FREQUENCY_MHZ = 160;
UWORD TIME_TO_SLEEP = 900; // seconds
UWORD count = 0;
UWORD countThreshold = 43200 / TIME_TO_SLEEP; // every ~12 hours

/* Layout ---------------------------------------------------------------------*/
// 10-286 (left 0-296 right, there is 10px margin each side)
// 0-128 (top 0-128 bottom, no margin)
Clock clockUI(75, 3);
MeasurmentDevice humidityMeter(10, 37, wi_humidity_32x32, 32, 32, 2);
MeasurmentDevice baroMeter(10, 62, wi_barometer_32x32, 32, 32, 3);
MeasurmentDevice thermoMeter(10, 94, wi_thermometer_32x32, 32, 32, 1);
void drawStaticImage() {
  Paint_DrawImage(img_flower_128x128, 0, 20, 128, 128); // image, y (0 == top), x, widht, height
};

/* Entry point ----------------------------------------------------------------*/
void setup() {
  // power-off unused components
  adc_power_off();
  WiFi.mode(WIFI_OFF);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  DEV_Module_Init(); // init pin modes and serial output
  EPD_2IN9_Init(EPD_2IN9_FULL); // reset esp32, init things
  EPD_2IN9_Clear(); // clear screen, turn display on | blinks
  DEV_Delay_ms(500);
  bme.begin(0x76);

  //Create a new image cache
  /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
  UWORD Imagesize = ((EPD_2IN9_WIDTH % 8 == 0)? (EPD_2IN9_WIDTH / 8 ): (EPD_2IN9_WIDTH / 8 + 1)) * EPD_2IN9_HEIGHT;
  if((ImageCache = (UBYTE *)malloc(Imagesize)) == NULL) {
    printf("Failed to apply for black memory...\r\n");
    while(1);
  }
  Paint_NewImage(ImageCache, EPD_2IN9_WIDTH, EPD_2IN9_HEIGHT, 270, WHITE);
  Paint_Clear(WHITE); // new image is b/w noise

  humidityMeter.draw();
  baroMeter.draw();
  thermoMeter.draw();
  drawStaticImage();

  setCpuFrequencyMhz(CPU_FREQUENCY_MHZ);
}

/* The main loop -------------------------------------------------------------*/
void loop() {
  ++count;
  if (count > countThreshold) { // EPD needs to be cleared every now and then
    count = 0;
    // EPD_2IN9_Init(EPD_2IN9_FULL) + EPD_2IN9_Clear() -- makes the screen blink
    EPD_2IN9_Init(EPD_2IN9_FULL); // reset esp32, init things
    EPD_2IN9_Clear(); // clear screen, turn display on | blinks

    humidityMeter.draw();
    baroMeter.draw();
    thermoMeter.draw();
    drawStaticImage();
  }
  EPD_2IN9_Init(EPD_2IN9_PART);

  // clockUI.draw(13, count * 3);
  // TODO batteryUI.update()
  humidityMeter.update();
  baroMeter.update();
  thermoMeter.update();

  EPD_2IN9_Display(ImageCache);
  DEV_Delay_ms(2000); 
  EPD_2IN9_Sleep();
  esp_light_sleep_start();
}
