#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIDTH 128
#define HEIGHT 64
#define OLED_RESET -1
#define SDA 35
#define SCL 34

extern int baud_list[];

Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, OLED_RESET);

void setup_OLED() {
Wire.begin(SDA, SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.display();
}

void menu_UART(int selected, char* uart_channel) {
    display.setCursor(0, 0);
    display.print("Select baud rate for ");
    display.print(uart_channel);
    display.setCursor(20, 40);
    display.print("<");
    display.setCursor(25, 40);
    display.print(baud_list[selected]);
    display.setCursor(40, 40);
    display.print(">");
    display.display();
}

void menu_msg(char* msg_A, char* msg_B) {
    display.setCursor(0, 0);
    display.println("UART Monitoring");
    display.setCursor(0, 10);
    display.print("UART_A: ");
    display.println(msg_A);
    display.setCursor(0, 20);
    display.print("UART_B: ");
    display.println(msg_B);
    display.display();
}

