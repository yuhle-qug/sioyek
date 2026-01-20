# Sioyek Enhancement Plan - Dictionary & Vocabulary Learning

## Tổng Quan
Cải tiến Sioyek với các tính năng học từ vựng bằng cách thêm:
1. Dictionary API integration (Định nghĩa tiếng Anh + tiếng Việt)
2. Gemini API để giải thích từ ngữ phức tạp
3. Vocabulary learning module
4. Quizlet export functionality

## Kiến Trúc Đề Xuất

```
sioyek/
├── pdf_viewer/               # Core C++ application
│   ├── src/
│   │   ├── config_manager.cpp    # Existing
│   │   ├── highlighting.cpp      # Existing - modify
│   │   └── ...
│   ├── CMakeLists.txt (hoặc .pro file)
│   └── ...
├── plugins/                       # New: Plugin system
│   ├── vocabulary_manager/
│   │   ├── vocabulary_manager.h
│   │   ├── vocabulary_manager.cpp
│   │   └── CMakeLists.txt
│   ├── dictionary_integration/
│   │   ├── dictionary_api.h
│   │   ├── dictionary_api.cpp
│   │   ├── english_dict_fetcher.h
│   │   ├── english_dict_fetcher.cpp
│   │   ├── vietnamese_dict_fetcher.h
│   │   ├── vietnamese_dict_fetcher.cpp
│   │   └── CMakeLists.txt
│   ├── gemini_integration/
│   │   ├── gemini_client.h
│   │   ├── gemini_client.cpp
│   │   └── CMakeLists.txt
│   └── quizlet_export/
│       ├── quizlet_exporter.h
│       ├── quizlet_exporter.cpp
│       └── CMakeLists.txt
├── resources/                      # UI resources
│   ├── dictionary_sidebar.qml      # Qt Quick for dictionary UI
│   ├── vocabulary_panel.qml        # Vocabulary learning panel
│   └── ...
└── scripts/
    ├── setup_plugins.py            # Setup script
    └── ...
```

## Các Tích Hợp API Đề Xuất

### 1. Dictionary APIs
- **Free Dictionary API** (Tiếng Anh): https://api.dictionaryapi.dev/api/v2/entries/en/{word}
- **Oxford Dictionary API** (Tiếng Anh) - Optional paid
- **Vietnamese Wiktionary**: https://vi.wiktionary.org/wiki/{word} (web scraping)
- **Từ Điển Song Ngữ Google Translate**: Integrating Google Translate API

### 2. Gemini API
- Google Gemini API để giải thích term phức tạp
- Tạo context-aware explanations

### 3. Quizlet Export Format
- CSV format: word, definition, vietnamese_translation
- Anki format support: .apkg file

## Implementation Steps

### Phase 1: Foundation
- [ ] Set up plugin architecture
- [ ] Create dictionary data structures
- [ ] Implement basic API fetching

### Phase 2: Dictionary Integration
- [ ] Integrate Free Dictionary API
- [ ] Add Vietnamese translation fetching
- [ ] Build dictionary UI sidebar
- [ ] Add caching mechanism

### Phase 3: Highlighting Enhancement
- [ ] Modify highlighting system to track selected words
- [ ] Add popup menu for dictionary lookup
- [ ] Store highlighted words for vocabulary learning

### Phase 4: AI Integration
- [ ] Integrate Gemini API
- [ ] Add term explanation feature
- [ ] Add context-based suggestions

### Phase 5: Vocabulary Learning
- [ ] Build vocabulary storage (SQLite)
- [ ] Create vocabulary review UI
- [ ] Implement learning statistics

### Phase 6: Quizlet Export
- [ ] Create Quizlet export format
- [ ] Implement batch export
- [ ] Add export templates

## Data Models

### Word Entry
```cpp
struct WordEntry {
    QString word;
    QString english_definition;
    QString vietnamese_translation;
    QString part_of_speech;
    QList<QString> examples;
    QDateTime highlighted_time;
    QString document_name;
    int page_number;
    float confidence_score;  // For AI-generated translations
};
```

### Vocabulary Database Schema
```sql
CREATE TABLE vocabulary (
    id INTEGER PRIMARY KEY,
    word TEXT UNIQUE,
    english_definition TEXT,
    vietnamese_translation TEXT,
    part_of_speech TEXT,
    examples TEXT,
    date_added DATETIME,
    times_reviewed INTEGER,
    learning_status TEXT,  -- 'new', 'learning', 'mastered'
    last_review_date DATETIME
);

CREATE TABLE document_highlights (
    id INTEGER PRIMARY KEY,
    word_id INTEGER,
    document_path TEXT,
    page_number INTEGER,
    highlight_time DATETIME,
    FOREIGN KEY(word_id) REFERENCES vocabulary(id)
);
```

## Technology Stack
- **Language**: C++ (core), Python (scripts)
- **GUI Framework**: Qt 5/6
- **HTTP Client**: Qt Network or curl
- **Database**: SQLite3
- **JSON**: nlohmann/json or Qt JSON
- **APIs**: 
  - Dictionary API (Free)
  - Google Translate/Gemini API
  - Custom scraping for Vietnamese sources

## Configuration Files

### keys_user.config additions
```
# Dictionary/Vocabulary Keybindings
highlight_and_lookup_dictionary=shift+d
show_vocabulary_panel=ctrl+shift+v
export_vocabulary_to_quizlet=ctrl+shift+e
ask_gemini_explanation=shift+g
```

### prefs_user.config additions
```
# Dictionary Settings
[dictionary]
enable_dictionary_sidebar=1
auto_fetch_definition=1
cache_definitions=1
cache_location=/path/to/cache

# AI Settings
[gemini]
enable_gemini=0
api_key=YOUR_API_KEY

# Vocabulary Settings
[vocabulary]
database_path=/path/to/vocabulary.db
auto_save=1
```

## Installation & Setup Instructions

1. Clone Sioyek repository
2. Apply enhancement patches
3. Install dependencies (Qt 5 dev, SQLite3, curl/Qt Network)
4. Build with CMake/qmake
5. Configure API keys in preferences
6. Start using enhanced features

## Future Enhancements
- Spaced repetition algorithm
- Integration with Anki
- Audio pronunciation
- Browser extension for web reading
- Sync across devices
- Statistics dashboard
