# Báo cáo Kỹ thuật & Đặc tả Module TV1: Khung Hiển Thị, Giao Diện Console & Khung Thông Tin

- **Thành viên phụ trách:** TV1 - Bảo
- **Môn học:** SS004 - Lớp R11
- **Phong cách thiết kế:** Tái hiện giao diện GUI cao cấp (Python Tetris style) trong Console Windows với Zero-Flicker Double Buffering và TrueColor 24-bit.
- **Đối tượng phối hợp & Review:** Review code cho TV2 (Đạt); TV5 (Nam) review code cho TV1.

---

## 1. Tổng quan nhiệm vụ của TV1

| STT | Hạng mục | Chi tiết thực hiện | Trạng thái |
|:---:|:---|:---|:---:|
| 1 | **Mô-đun cốt lõi** | - Xây dựng cấu trúc bàn cờ `Board` ($20 \times 15$).<br>- Hiện thực hàm `initBoard()` khởi tạo tường `#` và ô trống `' '`.<br>- Hiện thực hàm `draw()` kết xuất khung hình kết hợp bàn cờ và sidebar. | Hoàn thành |
| 2 | **Chức năng mở rộng** | - Tô màu console bằng Windows API (`SetConsoleTextAttribute` trong `windows.h`) kết hợp ANSI 24-bit TrueColor.<br>- Mỗi loại khối Tetromino có một màu riêng biệt phẳng lì nguyên khối `██`.<br>- Thiết kế khung Sidebar bên phải dạng thẻ bo góc mềm mại: Card **Score**, Card **Next**, Card **Level**. | Hoàn thành |
| 3 | **Mở rộng nâng cao** | - Khung tĩnh ô **Next Block** dạng Card xanh nổi bo góc `╭───────────╮`, bên trong chứa khối tiếp theo dạng khối đặc `██`.<br>- Khung tĩnh hiển thị **Score** và **Level** căn giữa chuẩn xác.<br>- Bảng hướng dẫn phím điều khiển (Controls Guide).<br>- Tối ưu kỹ thuật **Zero-Flicker**: Đóng gói toàn bộ khung hình vào bộ nhớ RAM và xuất trong 1 lệnh duy nhất. | Hoàn thành |
| 4 | **Báo cáo & Review** | - Soạn thảo Mục 1 báo cáo: Thông tin chung, Mục tiêu đề tài, Phân công nhiệm vụ.<br>- Thiết kế Mô hình kiến trúc phần mềm (`sections/architecture.tex`) với sơ đồ TikZ.<br>- Soạn thảo báo cáo Review mã nguồn chuyên sâu cho TV2 (`sections/review-tv2.tex`). | Hoàn thành |

---

## 2. Bố cục và Màu sắc chuẩn GUI Python Tetris

### 2.1 Bảng màu giao diện
- **Nền toàn cửa sổ:** Deep Navy Blue (`RGB: 30, 38, 82`).
- **Nền bàn cờ:** Dark Slate Charcoal (`RGB: 16, 20, 36`).
- **Lưới ô vuông:** Dấu chấm mờ `· ` (`RGB: 38, 48, 78`) chia đều từng ô vuông tỉ lệ 1:1.
- **Thẻ Card Sidebar:** Màu xanh lam nổi (`RGB: 45, 68, 140`) với đường viền bo góc mềm mại (`╭`, `─`, `╮`, `│`, `╰`, `╯`).
- **Khối gạch `██` đặc màu:**
  - Khối `I`: Cyan sáng (`RGB: 0, 205, 225`)
  - Khối `O`: Vàng sáng (`RGB: 240, 200, 0`)
  - Khối `T`: Tím hồng (`RGB: 170, 30, 220`)
  - Khối `S`: Xanh lá (`RGB: 0, 200, 60`)
  - Khối `Z`: Đỏ tươi (`RGB: 230, 40, 40`)
  - Khối `J`: Xanh dương đậm (`RGB: 25, 85, 235`)
  - Khối `L`: Cam rực rỡ (`RGB: 235, 110, 0`)
  - Khối bóng (Ghost): Texture lưới mờ `░░` (`RGB: 85, 95, 135`)

---

## 3. Kiểm thử đơn vị (Unit Tests)

Bộ kiểm thử đơn vị cho TV1 tại `tests/renderer_board_tests.cpp`:
- `testInitBoardCreatesCorrectBoundaries()`: Kiểm tra 4 đường biên `#` và các ô trống bên trong.
- `testInitBoardResetsBoard()`: Kiểm tra chức năng đặt khối, khóa khối và reset bàn cờ bằng `initBoard()`.
- `testRendererMethods()`: Kiểm tra kết xuất giao diện GUI với khối tiếp theo và tham số mặc định.

**Lệnh chạy kiểm thử:**
```bash
g++ -std=c++17 -Wall -Wextra src/Board.cpp src/Piece.cpp src/Renderer.cpp tests/renderer_board_tests.cpp -o build/test_renderer.exe
./build/test_renderer.exe
```
Kết quả: **All TV1 (Renderer & Board) unit tests passed successfully!**
