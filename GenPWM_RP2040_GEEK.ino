#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "hardware/pwm.h"

// Piny zgodne z Waveshare RP2040-GEEK
#define TFT_CS    9
#define TFT_DC    8
#define TFT_RST  12
#define TFT_BL   13

#define TFT_SCLK 10
#define TFT_MOSI 11

Adafruit_ST7789 tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);

// Piny PWM
#define PWM_PIN_2  2
#define PWM_PIN_3  3
#define PWM_PIN_28 28
#define PWM_PIN_29 29

float actual_freq;
float target_freq =1000000;

String unite;

void showFreq() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Generator PWM");
  tft.setCursor(10, 60);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("Freq:");

  tft.setCursor(10, 90);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(3);
  tft.print(actual_freq,3);
  tft.print(" ");
  tft.print(unite);
}

void setupFreq() {
  
  const float sys_clk = 100000000.0*2;
  // Zakładamy clkdiv = 1.0 jako start
  float clkdiv = 1.0;
  uint32_t wrap = sys_clk / (clkdiv * target_freq) - 1;
  // Jeśli wrap > 65535, musimy zwiększyć clkdiv
  while (wrap > 65535) {
    clkdiv += 0.1; // Zwiększamy z krokiem 0.1
    wrap = sys_clk / (clkdiv * target_freq) - 1;
  }
  // Zaokrągl wrap
  wrap = round(wrap);
  // Obliczamy końcową częstotliwość
  actual_freq = sys_clk / (clkdiv * (wrap + 1));
  if (actual_freq <999) {
    unite = "Hz";
  } else if(actual_freq > 999 && actual_freq < 999999){
    actual_freq =actual_freq /1000;
    unite = "kHz";
  }else if(actual_freq > 999999){
    actual_freq =actual_freq /1000000;
    unite = " MHz";
  }

  // Inicjalizacja PWM
  gpio_set_function(PWM_PIN_2, GPIO_FUNC_PWM);
  uint slice2 = pwm_gpio_to_slice_num(PWM_PIN_2);
  pwm_set_clkdiv(slice2, clkdiv);
  pwm_set_wrap(slice2, wrap);
  pwm_set_chan_level(slice2, PWM_CHAN_A, wrap / 2);
  pwm_set_enabled(slice2, true);
}


void setup() {
  Serial.begin(115200);
  delay(1500);

  // Włącz podświetlenie
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  pinMode(PIN3, OUTPUT);
  digitalWrite(PIN3, HIGH);

  // SPI1 pin init
  SPI1.setSCK(TFT_SCLK);
  SPI1.setTX(TFT_MOSI);

  // START: ustaw maksymalną częstotliwość SPI
  SPI1.begin(); // 100 MHz (często działa, ale zależy od wyświetlacza)
  delay(50);

  // Init wyświetlacza
  tft.init(135, 240);
  tft.setRotation(1);  
}


void loop() {
    if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    float freq = input.toFloat();

      if (freq > 0 && freq < 100000000) {  // np. max 1 MHz
        target_freq  = freq;
        Serial.print("Częstotliwoś ");
        Serial.println(target_freq);
        setupFreq();
        showFreq();
      } else {
        Serial.println("Podaj poprawną częstotliwość (np. 1000)");
      }
    }
}