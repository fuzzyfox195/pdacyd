# Cấu trúc và mở rộng Settings

Màn hình Settings dùng các nút nhỏ cùng kiểu với Start Screen: tiêu đề ở bên trái, icon 24×24 ở bên phải và màu icon/tiêu đề theo màu của setting. Danh sách được sắp theo thứ tự từ trái sang phải, rồi từ trên xuống dưới. Điều hướng quay lại dùng nút BACK vật lý; Sleep mode mở panel tùy chọn nguồn toàn màn hình.

Danh sách và thứ tự hiển thị được khai báo trong `src/apps/setting/app_setting.c`. Phần nội dung của từng setting nằm trong thư mục riêng:

```text
src/apps/setting/
├── connection/
├── theme/
├── ringtone/
├── battery/
├── time/
├── information/
└── i2c_scan/
```

Khi mở một mục, app thay nội dung trong vùng trang nhưng giữ màn hình Settings gốc. Nút BACK vật lý quay về danh sách; khi đang ở danh sách, nó đóng Settings và quay về Start Screen. Sleep mode mở một panel toàn màn hình và Cancel quay về đúng màn hình trước đó. Các setting chưa triển khai có trang khung với tiêu đề và nội dung chờ triển khai. Dùng `setting_option_ui_create()` trong `setting_option_ui.c` nếu trang mới cần cùng kiểu tiêu đề và mô tả; có thể thay lời gọi này bằng giao diện điều khiển riêng của setting.

## Thêm setting mới

1. Tạo thư mục `src/apps/setting/<setting_id>/`, cùng file header và source, ví dụ `wifi/setting_wifi.h` và `wifi/setting_wifi.c`.
2. Trong header, khai báo hàm tạo nội dung theo dạng `void setting_wifi_create_ui(lv_obj_t *parent);`. Source triển khai giao diện bên trong `parent`, ví dụ gọi `setting_option_ui_create(parent, "Wi-Fi", "Wi-Fi options will appear here.");`.
3. Include header mới trong `src/apps/setting/app_setting.c`.
4. Thêm icon bitmap vào `src/apps/setting/item_icon/`, khai báo symbol `extern const lv_img_dsc_t icon_wifi;` trong `app_setting.c`, rồi thêm một phần tử vào mảng `setting_options` tại vị trí muốn hiển thị, ví dụ `{ "Wi-Fi", &icon_wifi, 0x2583E8, setting_wifi_create_ui, NULL }`. Mảng này quyết định thứ tự trái sang phải, trên xuống dưới. Icon và tiêu đề được tô theo màu RGB 24-bit của setting. Tham số cuối là hàm dọn tài nguyên khi rời trang; dùng `NULL` nếu không cần dọn.

Không thêm setting vào `APP_REGISTRY`: các setting là trang con của app `app_setting`, không phải app độc lập. Grid tự tính vị trí và chiều cao cuộn theo số phần tử trong `setting_options`.

## I2C Scan

`src/apps/setting/i2c_scan/setting_i2c_scan.cpp` quét bus I2C khi mở trang. Mỗi địa chỉ được kiểm tra theo timer LVGL để giao diện vẫn phản hồi; spinner hiển thị trong lúc quét. Danh sách đã biết được đọc từ `/data/i2c.json` trên thẻ SD theo cấu trúc `i2c` trong [sd_card_structure.md](sd_card_structure.md). Địa chỉ đang kết nối có trong file sẽ hiện cùng part code ở bảng I2C; các địa chỉ còn lại sẽ hiện trong cột Unknown. Trang được quét lại mỗi lần mở.

Nếu chưa có `/data/i2c.json` hoặc không đọc được file, các địa chỉ tìm thấy vẫn hiện trong Unknown và trang báo file chưa khả dụng.

## Sleep mode

Mục này mở panel Sleep Options dùng chung với nút Sleep vật lý. Panel có Modem sleep, Light sleep, Deep sleep và Cancel. Chi tiết nút cứng và nguồn wake nằm trong [physical_buttons_sleep.md](physical_buttons_sleep.md).

## Button Debug

`src/apps/setting/button_debug/setting_button_debug.cpp` shows the live stable input state of the PCF8575 BACK, HOME, OPTION, and FAVORITE pins. The view polls the input service's cached state and does not perform additional I2C reads. If the expander is unavailable, all button states show `--`.
