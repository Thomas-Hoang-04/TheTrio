# Thiết bị giám sát và can thiệp đường truyền UART

Mô tả

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
  - Việc lưu trữ được diễn ra tự động sau một khoảng thời gian nhất định kể từ lần cuối người dùng thao tác với hệ thống _(sử dụng Timer để định thời gian)_
  - Dữ liệu trong EEPROM sẽ được trích xuất và cài làm mặc định cho các kênh UART mỗi khi thiết bị khởi động lại

### ĐẶC TẢ HÀM

- Giải thích một số hàm quan trọng: ý nghĩa của hàm, tham số vào, ra

  ```C
     /**
      *  Hàm tính ...
      *  @param  x  Tham số
      *  @param  y  Tham số
      */
     void abc(int x, int y = 2);
  ```

### KẾT QUẢ

- Video demo sản phẩm: [_**Thiết bị giám sát đường truyền UART**_](https://youtu.be/B4v0FH4aFU4)
