# Sioyek Enhanced - Vocabulary Learning Plugin Setup

Hướng dẫn thiết lập và sử dụng tính năng học từ vựng tiếng Anh được tích hợp vào Sioyek.

## Tính năng

- **Tra cứu từ tự động**: Khi bạn highlight một từ, ứng dụng tự động tra cứu định nghĩa tiếng Anh + dịch tiếng Việt
- **Panel định nghĩa**: Hiển thị sidebar bên phải với:
  - Từ (kèm phát âm)
  - Định nghĩa tiếng Anh
  - Dịch tiếng Việt
  - Giải thích thêm từ Gemini AI (nếu cấu hình)
- **Lưu từ vựng**: Tự động lưu các từ được highlight vào cơ sở dữ liệu SQLite
- **Xuất Quizlet**: Xuất danh sách từ vựng sang định dạng CSV/HTML để import vào Quizlet, Anki, Memrise

## Cấu hình

### 1. Thiết lập API Key (tùy chọn cho Gemini)

Nếu muốn sử dụng Gemini AI để giải thích thêm từ vựng:

```bash
# Windows PowerShell
$env:GEMINI_API_KEY = "your-api-key-here"

# Hoặc thêm vào .env (nếu sử dụng)
# GEMINI_API_KEY=your-api-key-here
```

Lấy API key tại: https://ai.google.dev/

### 2. Cấu hình ứng dụng (prefs_user.config)

Thêm các tùy chọn vào `pdf_viewer/prefs_user.config`:

```ini
# Enable/disable auto word lookup on highlight
vocab_auto_lookup = true

# Dictionary source: "free_dictionary" or "wiktionary"
vocab_dictionary_source = free_dictionary

# Enable Gemini explanations
vocab_use_gemini = true

# Panel position: "right" or "bottom"
vocab_panel_position = right

# Panel width (in pixels)
vocab_panel_width = 380

# Panel height (in pixels)
vocab_panel_height = 300
```

### 3. Keymap (keys_user.config)

Thêm phím tắt để xuất Quizlet:

```ini
q export_quizlet
```

Hoặc bất kỳ phím nào bạn thích.

## Xây dựng (Build)

### Yêu cầu

- **Qt5** hoặc **Qt6** (với development headers)
- **C++17 compiler**
- **MuPDF** (phần phụ thuộc hiện có)
- **CMake** hoặc **qmake** (tùy theo cách build)

### Trên Windows (với Qt6 + MSVC)

```bash
# 1. Đặt biến môi trường (nếu cần)
$env:GEMINI_API_KEY = "your-key"

# 2. Gọi qmake để sinh Makefile/VS project
qmake pdf_viewer_build_config.pro -o Makefile

# 3. Build
nmake  # hoặc mingw32-make nếu dùng MinGW

# Hoặc nếu qmake sinh VS project
msbuild sioyek.sln /property:Configuration=Release
```

### Trên Linux

```bash
qmake pdf_viewer_build_config.pro
make
sudo make install
```

### Trên macOS

```bash
qmake pdf_viewer_build_config.pro
make
```

## Sử dụng

### 1. Highlight một từ

- Chọn chế độ **select highlight** (phím tắt tùy theo config)
- Double-click hoặc kéo để chọn một từ
- Thả chuột → từ được highlight + panel tra cứu bật ở cạnh phải

### 2. Xem định nghĩa

Panel hiển thị:
- **Word**: từ + phát âm (nếu có)
- **POS**: Từ loại (noun, verb, etc.)
- **EN**: Định nghĩa tiếng Anh (từ Free Dictionary API)
- **VI**: Dịch tiếng Việt
- **Gemini**: Giải thích từ Gemini (nếu bật)

### 3. Quản lý từ vựng

Các từ đã highlight được lưu vào `vocabulary.db` tự động.

Xem thông tin:
- Page được highlight
- Tài liệu nguồn
- Ngày thêm

### 4. Xuất Quizlet

- Nhấn `q` (hoặc phím bạn đã map)
- Chọn thư mục lưu file CSV
- File `vocabulary.csv` chứa tất cả từ với định nghĩa

**Import vào Quizlet**:
1. Truy cập https://quizlet.com/create
2. Chọn "Import from a file"
3. Upload file CSV
4. Format: `Term,Definition`

**Import vào Anki**:
1. Tạo deck mới
2. Tools → Import
3. Chọn file `vocabulary.csv`
4. Quizlet cũng hỗ trợ export sang Anki format

**Import vào Memrise**:
1. Tạo course
2. Import từ CSV
3. Cùng format 2 cột

## Cấu trúc dự án

```
plugins/
├── dictionary_integration/
│   ├── dictionary_api.h          (abstract interface)
│   ├── english_dict_fetcher.h    (Free Dictionary API + MyMemory translation)
│   └── english_dict_fetcher.cpp
├── gemini_integration/
│   ├── gemini_client.h           (Google Generative Language API)
│   └── gemini_client.cpp
├── quizlet_export/
│   ├── quizlet_exporter.h        (CSV/HTML/Anki export)
│   └── quizlet_exporter.cpp
└── vocabulary_manager/
    ├── vocabulary_manager.h      (SQLite database for vocabulary)
    ├── vocabulary_manager.cpp
    ├── vocab_integration.h       (main integration point)
    └── vocab_integration.cpp     (hooks highlight → lookup → panel)
```

## API References

### Free Dictionary API
- Endpoint: `https://api.dictionaryapi.dev/api/v2/entries/en/{word}`
- Tính năng: Định nghĩa tiếng Anh, phát âm, ví dụ
- Giới hạn: Miễn phí, không cần API key

### MyMemory Translation API
- Endpoint: `https://api.mymemory.translated.net/get`
- Tính năng: Dịch EN → VI
- Giới hạn: Miễn phí, ~100 request/ngày

### Google Generative Language API (Gemini)
- Model: `gemini-pro`
- Tính năng: Giải thích, dịch, tạo ghi chú học tập
- Giới hạn: 60 request/phút (free tier)
- Yêu cầu: API key từ https://ai.google.dev/

## Troubleshooting

### Panel không hiện
- Kiểm tra `vocab_auto_lookup = true` trong config
- Đảm bảo highlight một từ duy nhất (không có khoảng trắng)
- Xem console log để tìm lỗi

### Tra cứu thất bại
- Kiểm tra kết nối internet
- Free Dictionary API có thể không tìm thấy từ chuyên biệt
- MyMemory có thể bị rate limit

### Gemini không hoạt động
- Kiểm tra `$env:GEMINI_API_KEY` đã set đúng
- Xác nhận API key hợp lệ
- Giới hạn 60 request/phút (wait 1 phút nếu limit)

### Export CSV lỗi
- Đảm bảo thư mục đích tồn tại và có quyền ghi
- File name không chứa ký tự đặc biệt

## Tương lai

- [ ] Giao diện đầy đủ hơn (tombol Add/Remove từ thêm study set)
- [ ] Spaced repetition quiz trong ứng dụng
- [ ] Context-aware explanation (dựa trên đoạn văn xung quanh)
- [ ] Hỗ trợ nhiều ngôn ngữ
- [ ] Đồng bộ với Anki/Quizlet online

## Hỗ trợ

Báo lỗi hoặc đề xuất tính năng tại: [GitHub Issues](https://github.com/ahrm/sioyek)
