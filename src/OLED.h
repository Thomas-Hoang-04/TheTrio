#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIDTH 128
#define HEIGHT 64
#define OLED_RESET -1
#define SDA 13
#define SCL 14

extern Adafruit_SSD1306 display;

void setup_OLED();
void menu_UART(int selected, String uart_channel);
void menu_UART_A(String msg_A);
void menu_UART_B(String msg_B);

#endif
