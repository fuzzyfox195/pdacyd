# Thêm app mới

Mỗi app tự khai báo tên, ID, thông tin tile, vị trí và vòng đời trong thư mục riêng dưới `src/apps/`. Start Screen lấy danh sách từ registry; không cần sửa file trong `src/ui/` khi thêm app.

## Các bước

1. Tạo `src/apps/<app_id>/` với file header và source cho app. Thêm icon source vào cùng thư mục nếu app có icon.
2. Định nghĩa `const app_descriptor_t app_<symbol>` trong source. Khai báo các trường `.app_id`, `.app_name`, `.start_tile`, và callback vòng đời.
3. Trong `.start_tile`, đặt icon, màu, kiểu `APP_TILE_SIZE_SMALL` hoặc `APP_TILE_SIZE_LARGE`, tọa độ `.x` và `.y`, cùng `title` và `content` nếu tile lớn.
4. Thêm một dòng `X(app_<symbol>)` vào `APP_REGISTRY` trong `src/apps/app_registry.h`.

Ví dụ metadata tile:

```c
.start_tile = {
    .icon = &example_icon,
    .color_hex = 0x2583E8,
    .size = APP_TILE_SIZE_LARGE,
    .x = 8,
    .y = 0,
    .title = "Preview title",
    .content = "Preview description",
},
```

Các source dưới `src/` được PlatformIO tự biên dịch. `app_registry.h` là điểm đăng ký thủ công duy nhất: nó vừa tạo registry Application Manager vừa xác định thứ tự hiển thị trên Start Screen. Metadata, chuỗi và icon được khai báo tĩnh để không cần cấp phát RAM cho cấu hình app. Kích thước tile, lề và khoảng cách do Start Screen quản lý chung; vị trí từng tile đặt trong `.start_tile` của app.

ID app phải duy nhất. Để ẩn nội dung preview của tile nhỏ, dùng kiểu `APP_TILE_SIZE_SMALL` và để `title`, `content` là `NULL`.

## Lỗi thường gặp khi thêm app

Khi thêm app vào `APP_REGISTRY`, symbol của descriptor trong `X(...)` phải trùng chính xác với tên biến được khai báo bằng `const app_descriptor_t` trong file source.

Ví dụ, dòng đăng ký:

```c
X(app_system_log)
```

cần có descriptor:

```c
const app_descriptor_t app_system_log = {
    /* ... */
};
```

Nếu source chỉ định nghĩa `const app_descriptor_t system_log`, registry lại khai báo `app_system_log`, linker sẽ không tìm thấy symbol `app_system_log` và build thất bại với lỗi `undefined reference to app_system_log`. Sửa tên descriptor trong source để khớp với tên trong `APP_REGISTRY`, đồng thời đảm bảo header của app export đúng tên symbol đó.
