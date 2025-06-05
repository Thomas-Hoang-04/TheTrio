#include <OLED.h>

Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, OLED_RESET);

void setup_OLED() {
  Wire.begin(SDA, SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
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

void menu_msg(String msg_A, String msg_B) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("UART Monitoring");
    display.print("UART_A: ");
    display.println(msg_A);
    display.setCursor(0, 35);
    display.print("UART_B: ");
    display.println(msg_B);
    display.display();
}