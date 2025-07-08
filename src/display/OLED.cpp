#include "display/OLED.h"

Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, OLED_RESET);

void setup_OLED() {
  Wire.begin(SDA, SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    return;
  }
  display.setTextWrap(true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.display();
}

void menu_UART(int baud_rate, String uart_channel) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Select baud rate for ");
    display.setCursor(40, 18);
    display.println(uart_channel);
    display.setCursor(20, 40);
    display.print("<");
    display.setCursor(40, 40);
    display.print(baud_rate);
    display.setCursor(100, 40);
    display.print(">");
    display.display();
}

void menu_msg(String msg_A, String msg_B, int baud_rate_A, int baud_rate_B) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("UART Monitoring");

    display.setCursor(0, 12);
    display.print("A:");
    display.print(baud_rate_A);

    display.setCursor(64, 12);
    display.print("B:");
    display.print(baud_rate_B);

    display.setCursor(0, 24);
    display.print("A:");
    display.println(truncateMessage(msg_A, MESSAGE_DISPLAY_LENGTH));
    
    display.setCursor(0, 36);
    display.print("B:");
    display.println(truncateMessage(msg_B, MESSAGE_DISPLAY_LENGTH));

    display.display();
}

String truncateMessage(String message, int maxLength) {
    if (message.length() <= maxLength) {
        return message;
    }
    return message.substring(0, maxLength - 3) + "...";
}