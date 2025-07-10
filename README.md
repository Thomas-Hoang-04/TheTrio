# Thiết bị giám sát và can thiệp đường truyền UART

_Từ ESP32 tạo ra thiết bị có khả năng giám sát và can thiệp đường truyền UART. Có thể sử dụng trong thực tế cho các trường hợp: giải mã giao thức của các thiết bị điện tử đóng (ti vi, máy giặt); chẩn đoán và sửa lỗi sai của thiết bị điện tử, tạo ra hệ thống giám sát capture và debug realtime._

_Báo cáo dưới đây tập trung vào việc mô tả chi tiết phần cứng và phần mềm của sản phẩm, tính năng của sản phẩm và kết quả đạt được._

## GIỚI THIỆU

**Đề bài**: _Xây dựng môt thiết bị giám sát và can thiệp đường truyển UART, cho phép theo dõi các thông tin được truyền đi giữa các thiết bị giao tiếp qua giao thức UART. Thiết bị có khả năng điều chỉnh và lưu trữ baud rate giao tiếp của các kênh truyền để phù hợp với nhiều yêu cầu khác nhau của nhiều loại thiết bị_

**Tính năng sản phẩm:**

- Cho phép kết nối tới tối đa 2 thiết bị giao tiếp UART để thiết lập giám sát, hỗ trợ giao diện USB _(thông qua 2 mạch USB-to-UART)_ để tăng khả năng tương thích với nhiều chủng loại thiết bị
- Hiển thị các thông tin được trao đổi giữa 2 kênh UART thông qua màn hình OLED
- Cho phép thay đổi tốc độ baud rate của các kênh UART để thích ứng với yêu cầu của các thiết bị được kết nối
- Hỗ trợ lưu trữ cấu hình baud rate gần nhất được sử dụng để thiết lập giao tiếp UART

**Ảnh chụp minh họa:**

  ![Ảnh minh họa]()

## TÁC GIẢ

- Tên nhóm: **The Trio**
- Thành viên trong nhóm
  |STT|Họ tên|MSSV|Công việc|
  |--:|--|--|--|
  |1|Vũ Ngọc Tiến Dũng|20225182|Xử lý UART, xử lý ngắt nút bấm|
  |2|Hoàng Minh Hải|20225187|Hiển thị OLED, thiết lập phần cứng|
  |3|Phạm Việt Hòa|20225128|Xử lý bộ nhớ flash|

## MÔI TRƯỜNG HOẠT ĐỘNG

_**Linh kiện sử dụng**_

- ESP32 Dev Kit
- Màn hình OLED SSD1306 0.96 inch
- Cổng chuyển đổi USB-to-UART (x2)
- IC 74HC14 (Hex Inverting Schmitt Trigger)
- Linh kiện thụ động: tụ điện, điện trở, nút bấm

## SƠ ĐỒ SCHEMATIC

![UART Interceptor Schematic](./UART%20Schematic.png)

### TÍCH HỢP HỆ THỐNG

**1. Phần cứng**
- ESP32 làm nhiệm vụ xử lý và chuyển tiếp thông tin được trao đổi giữa 2 kênh UART, hiển thị ra màn hình OLED, thiết lập và lưu trữ cấu hình baud rate cho các kênh UART
- Hệ thống nút bấm giúp người dùng cài đặt baud rate cho các kênh UART được kết nôi
- Mạch RC cho từng nút bấm, kết hơp với IC 74HC14, giúp xử lý hiện tượng nảy phím của các nút bấm, giảm thao tác xử lý cho ESP32
- Các mạch USB-to-UART có nhiệm vụ kết nối với các thiết bị ngoại vi qua cổng USB, tiếp nhận thông tin và chuyển về cho ESP32 qua kết nối UART để xử lý
- Màn hình OLED hiển thị các thông tin trao đổi giữa các kênh UART, đồng thời hiển thị các thông số khi người dùng tiến hành thiết lập baud rate cho các kênh UART

**2. Phần mềm**

_Ứng dụng trên ESP32 thực hiện các chức năng bao gồm_

- Bắt và xử lý các bản tin nhận được từ 2 kênh UART ngoại vi, cho hiển thị ra màn hình OLED để tiện theo dõi
- Chuyển tiếp các bản tin nhận được từ UART_A sang UART_B và ngược lại
- Xử lý tín hiệu ngắt từ nút bấm ứng với các chức năng như chọn kênh UART để thao tác, tăng/giảm baud rate.
- Lưu dữ liệu baud rate của từng kênh truyền _(nếu có thay đổi)_ vào trong bộ nhớ flash (EEPROM) của ESP32.
  - Việc lưu trữ được diễn ra tự động sau một khoảng thời gian nhất định kể từ lần cuối người dùng thao tác với hệ thống _(sử dụng Hardware Timer để định thời gian chính xác)_
  - Dữ liệu trong EEPROM sẽ được trích xuất và cài làm mặc định cho các kênh UART mỗi khi thiết bị khởi động lại

### ĐẶC TẢ HÀM

- Hàm xử lý giao diện trên màn hình OLED

  ```C
  /**
   * @brief Hàm hiển thị menu thiết lập baud rate cho kênh UART
   *
   * @param baud_rate: baud rate được chọn
   * @param uart_channel: tên kênh UART
   */
  void menu_UART(int baud_rate, String uart_channel);

  /**
   * @brief Hàm tạo giao diện hiển thị thông tin trao đổi giữa 2 kênh UART
   *
   * @param msg_A: thông tin gửi từ UART_A
   * @param msg_B: thông tin gửi từ UART_B
   */
  void menu_msg(String msg_A, String msg_B);
  ```

- Hàm xử lý bộ nhớ flash

  ```C
  /**
   * @brief Hàm thực hiện lưu trữ cấu hình baud rate của các kênh UART vào bộ nhớ flash
   *
   * @param baudIndex1: chỉ số của baud rate được chọn cho kênh UART_A
   * @param baudIndex2: chỉ số của baud rate được chọn cho kênh UART_B
   *
   * @note Hàm sẽ chỉ thực hiện nếu phát hiện thay đổi về cấu hình baud rate của các kênh UART
   * @note Các chỉ số ứng với vị trí của baud rate trong mảng được khai báo tại file main.cpp
   */
  void saveBaudRates(int baudIndex1, int baudIndex2);

  /**
   * @brief Hàm thực hiện đọc cấu hình baud rate của các kênh UART từ bộ nhớ flash
   *
   * @param baudIndex1: con trỏ lưu chỉ số của baud rate được chọn cho kênh UART_A
   * @param baudIndex2: con trỏ lưu chỉ số của baud rate được chọn cho kênh UART_B
   *
   * @note Các chỉ số ứng với vị trí của baud rate trong mảng được khai báo tại file main.cpp
   */
  void loadBaudRates(int* baudIndex1, int* baudIndex2);

  /**
   * @brief Hàm thực hiện kiểm tra và lưu trữ cấu hình baud rate của các kênh UART
   *
   * Hàm được khai báo trong file main.cpp, gọi tới hàm saveBaudRates() để thực hiện lưu trữ cấu hình baud rate của các kênh UART và in log ra Serial
   */
  void checkAndSaveBaudRates();
  ```

- Hàm xử lý tín hiệu từ nút bấm

  ```C
  /**
   * @brief Hàm xử lý tín hiệu từ nút bấm
   *
   * Hàm sẽ xử lý các cờ tín hiệu thiết lập bởi ngắt để thực hiện các chức năng như chọn kênh UART để thao tác, tăng/giảm baud rate.
   * Hàm được thiết kế thực hiện trong main loop để giảm tải cho ISR
   */
  void processButtonAction();

  /**
   * @brief Hàm xử lý ngắt từ nút bấm
   *
   * Hàm sẽ kích hoạt khi có tín hiệu ngắt từ nút bấm, thiết lập các cờ tương ứng để hàm xử lý trong main loop thực hiện các chức năng tương ứng.
   *
   * Hàm cũng sẽ kích hoạt timer để tự động phát tín hiệu thực hiện lưu trữ cấu hình baud rate của các kênh UART (nếu có thay đổi) sau 3s kể từ lần cuối người dùng thao tác với hệ thống
   *
   * Hàm được đặt trong IRAM để đảm bảo tốc độ cho ISR
   */
  void IRAM_ATTR handleButtonInterrupt();
  ```

- Hàm xử lý thông tin trao đổi giữa 2 kênh UART

  ```C
  /**
   * @brief Hàm xử lý thông tin trao đổi giữa 2 kênh UART
   *
   * Hàm sẽ tiếp nhận thông tin từ 2 kênh UART và thực hiện chuyển tiếp
   * Hàm sẽ lưu thông tin nhận được từ 2 kênh UART vào biến msg_A và msg_B để hiển thị trên màn hình OLED
   */
  void handleUARTCommunication();
  ```

### KẾT QUẢ

- Video demo sản phẩm: [_**Thiết bị giám sát đường truyền UART**_](https://youtu.be/B4v0FH4aFU4)
