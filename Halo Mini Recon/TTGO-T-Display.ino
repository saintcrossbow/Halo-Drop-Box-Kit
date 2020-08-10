// Adapted from https://github.com/Xinyuan-LilyGO/TTGO-T-Display/blob/master/TTGO-T-Display.ino

#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include <Button2.h>
#include "esp_adc_cal.h"
#include "bmp.h"

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23

#define TFT_BL              4   // Display backlight control pin
#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
#define BUTTON_1            35
#define BUTTON_2            0

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

char buff[512];
String tsid;
int vref = 1100;
int btnCick = false;

#define ENABLE_SPI_SDCARD

//Uncomment will use SDCard, this is just a demonstration,
//how to use the second SPI
#ifdef ENABLE_SPI_SDCARD

#include "FS.h"
#include "SD.h"

SPIClass SDSPI(HSPI);

#define MY_CS       33
#define MY_SCLK     25
#define MY_MISO     27
#define MY_MOSI     26

void setupSDCard()
{
    SDSPI.begin(MY_SCLK, MY_MISO, MY_MOSI, MY_CS);
    //Assuming use of SPI SD card
    if (!SD.begin(MY_CS, SDSPI)) {
        Serial.println("Card Mount Failed");
// We don't care about this part
//        tft.setTextColor(TFT_RED);
//        tft.drawString("SDCard Mount FAIL", tft.width() / 2, tft.height() / 2 - 32);
//        tft.setTextColor(TFT_GREEN);
    } else {
        tft.setTextColor(TFT_GREEN);
        Serial.println("SDCard Mount PASS");
        tft.drawString("SDCard Mount PASS", tft.width() / 2, tft.height() / 2 - 48);
        String size = String((uint32_t)(SD.cardSize() / 1024 / 1024)) + "MB";
        tft.drawString(size, tft.width() / 2, tft.height() / 2 - 32);
    }
}
#else
#define setupSDCard()
#endif


void wifi_scan();
void wifi_landscape();

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_light_sleep_start();
}

void showVoltage()
{
    static uint64_t timeStamp = 0;
    if (millis() - timeStamp > 1000) {
        timeStamp = millis();
        uint16_t v = analogRead(ADC_PIN);
        float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
        String voltage = "Voltage :" + String(battery_voltage) + "V";
        Serial.println(voltage);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(voltage,  tft.width() / 2, tft.height() / 2 );
    }
}

void button_init()
{
    btn1.setPressedHandler([](Button2 & b) {
        Serial.println("Detect Voltage..");
        btnCick = true;
    });

    btn2.setPressedHandler([](Button2 & b) {
        btnCick = false;
        Serial.println("btn press wifi scan");
        wifi_scan();
    });
}

void button_loop()
{
    btn1.loop();
    btn2.loop();
}

void wifi_landscape()
{
    String wifi1 = "";
  
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);

    tft.drawString("[ Graphing... ]", tft.width() / 2, tft.height() / 2);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int16_t n = WiFi.scanNetworks();
    tft.fillScreen(TFT_BLACK);
      if (n == 0) {
          tft.drawString("no networks found", tft.width() / 2, tft.height() / 2);
      } else {
          tft.setTextDatum(TL_DATUM);
          tft.setCursor(0, 0);
          Serial.printf("Found %d net\n", n);
          for (int ch = 1; ch < 16; ++ch)
          {
            wifi1 = "Ch";
            if (ch < 10) 
            {
              wifi1 = wifi1 + "0";
            }
            tft.setTextColor(TFT_CYAN);
            wifi1 = wifi1 + ch + ": ";
            tft.print(wifi1);
            for (int i = 0; i < n; ++i) {              
              //    https://github.com/espressif/arduino-esp32/issues/2928
              switch (WiFi.encryptionType(i)) {
                case (0):
                  tft.setTextColor(TFT_RED);  //OPEN
                  break;
                case (1):
                  tft.setTextColor(TFT_BLUE);  //WEP
                  break;
                case (3):
                  tft.setTextColor(TFT_GREEN); 
                  break;
                case (4):
                  tft.setTextColor(TFT_GREEN); 
                  break;
                case (5): //enterprise
                  tft.setTextColor(TFT_CYAN); 
                  break;
                case (6): //max
                  tft.setTextColor(TFT_CYAN); 
                  break;
                default:
                  tft.setTextColor(TFT_YELLOW);  // Unknown
                  break;
                }
                if (WiFi.channel(i) == ch) {
                  tft.print("*");
                }
            }
            tft.println("");
          }
      }
    WiFi.mode(WIFI_OFF);
    espDelay(1000);
}


void wifi_scan()
{
    String wifi1 = "";
    String ch = "";
  
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);

    tft.drawString("[ Scanning ... ]", tft.width() / 2, tft.height() / 2);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int16_t n = WiFi.scanNetworks();
    tft.fillScreen(TFT_BLACK);
      if (n == 0) {
          tft.drawString("no networks found", tft.width() / 2, tft.height() / 2);
      } else {
          tft.setTextDatum(TL_DATUM);
          tft.setCursor(0, 0);
          Serial.printf("Found %d net\n", n);
          for (int i = 0; i < n; ++i) {

            //    https://github.com/espressif/arduino-esp32/issues/2928
            switch (WiFi.encryptionType(i)) {
              case (0):
                tft.setTextColor(TFT_RED);  //OPEN
                break;
              case (1):
                tft.setTextColor(TFT_BLUE);  //WEP
                break;
              case (3):
                tft.setTextColor(TFT_GREEN); 
                break;
              case (4):
                tft.setTextColor(TFT_GREEN); 
                break;
              case (5): //enterprise
                tft.setTextColor(TFT_CYAN); 
                break;
              case (6): //max
                tft.setTextColor(TFT_CYAN); 
                break;
              default:
                tft.setTextColor(TFT_YELLOW);  // Unknown
                break;
              }
            wifi1 = "";
            wifi1 = wifi1 + WiFi.RSSI(i) + "dBm ";          
            wifi1 = wifi1 + "Ch";        
            ch = WiFi.channel(i);   
            if (ch.length() < 2) 
            {
              wifi1 = wifi1 + "0";
            }
            wifi1 = wifi1 + ch + " ";   
            wifi1 = wifi1 + WiFi.BSSIDstr(i) + " ";         
            wifi1 = wifi1 + WiFi.SSID(i);             // SSID
            if ( wifi1.length() > 40 ) {
              wifi1 = wifi1.substring(0,40);
            }
            tft.println(wifi1);
          }
      }
    WiFi.mode(WIFI_OFF);
}

//https://blog.robberg.net/wifi-scanner-with-esp32/
String translateEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case (0):
      return "OPEN";
    case (1):
      return "WEP";
    case (2):
      return "WPA_PSK";
    case (3):
      return "WPA2_PSK";
    case (4):
      return "WPA?_PSK";
    case (5):
      return "WPA2_ENTPR";
    default:
      return "UNKNOWN";
    }
  }

void setup()
{
    Serial.begin(115200);
    Serial.println("Start");

    /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */
    pinMode(ADC_EN, OUTPUT);
    digitalWrite(ADC_EN, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);

    if (TFT_BL > 0) {                           // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
        pinMode(TFT_BL, OUTPUT);                // Set backlight pin to output mode
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    }

    tft.setSwapBytes(true);
    tft.pushImage(0, 0,  240, 135, ttgo);
    espDelay(500);


    tft.setRotation(1);

    button_init();

    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
    } else {
        Serial.println("Default Vref: 1100mV");
    }


    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);

    setupSDCard();

    tft.setTextColor(TFT_GREEN); 
    tft.drawString("[ WiFi Landscape ]", tft.width(), 16);
    tft.drawString("[ WiFi Scanner ]", tft.width(), tft.height() -16  );
    tft.setTextColor(TFT_BLUE); 
    tft.drawString("AP Verification and Quick Scan", 16, tft.height() / 2 - 16);
    tft.setTextColor(TFT_CYAN); 
    tft.drawString("Halo Drop Box Kit", 16, tft.height() / 2);
    tft.drawString("2020 Aug 07", 16, tft.height() / 2 + 16);
    tft.setTextDatum(TL_DATUM);
}

void loop()
{
    if (btnCick) {
        wifi_landscape();  
        btnCick = false;      
    }
    button_loop();
}
