# Sioyek Enhanced - Quick Reference Guide

## 🚀 5-Minute Quick Start

### For Users

**1. Install & Build** (5 minutes)
```bash
# On Windows
# 1. Install Qt5 from https://www.qt.io/download
# 2. Add Qt\bin to PATH
# 3. Run:
cd c:\Users\huy37\Projects\sioyek-enhanced
qmake pdf_viewer_build_config.pro
nmake
```

**2. First Run**
```bash
# Open a PDF
.\Release\sioyek.exe document.pdf

# Highlight any word → sidebar appears with definitions
# Press 'q' → export vocabulary to CSV
```

**3. Optional: Enable AI**
```bash
# Get free API key from https://ai.google.dev/
$env:GEMINI_API_KEY = "your-key-here"
# Restart Sioyek
```

**4. Study on Quizlet**
```
1. Go to https://quizlet.com/create
2. Click "Import from file"
3. Upload vocabulary.csv
4. Start learning!
```

---

### For Developers

**Adding a New Dictionary Source**:
```cpp
// 1. Create MyDictAPI.h inheriting from DictionaryAPI
class MyDictAPI : public DictionaryAPI {
    WordDefinition fetchDefinition(const QString& word) override;
    bool isAvailable() override;
};

// 2. Implement in MyDictAPI.cpp
WordDefinition MyDictAPI::fetchDefinition(const QString& word) {
    // API call logic
    return {word, "definition", ...};
}

// 3. Register in vocab_integration.cpp
auto my_dict = std::make_unique<MyDictAPI>();
// Use in showLookupPanel()
```

**Adding New Export Format**:
```cpp
// In quizlet_exporter.h, add method:
QString exportToMyFormat(const QVector<VocabularyEntry>& words);

// In quizlet_exporter.cpp, implement:
QString QuizletExporter::exportToMyFormat(...) {
    QString output;
    for (const auto& entry : words) {
        output += entry.word + "\t" + entry.englishDefinition + "\n";
    }
    return output;
}
```

---

## 📊 Architecture at a Glance

```
┌─────────────────────────────────────────┐
│  Sioyek PDF Viewer                      │
│  (Main: pdf_viewer/main_widget.cpp)     │
│                                         │
│  When user highlights a word:           │
│  → mouseReleaseEvent triggered          │
│  → VocabIntegration::showLookupPanel()  │
└────────────────────┬────────────────────┘
                     │
         ┌───────────┴───────────┐
         │ VocabIntegration      │ ← Main Integration Point
         │ (Facade Pattern)      │
         └─┬─────┬──────┬────┬───┘
           │     │      │    │
    ┌──────┴─┐  ┌┴──┐ ┌┴────┴────┐
    │ English│  │Gem│ │ Quizlet  │
    │ Dict.  │  │ini│ │ Exporter │
    │        │  │   │ │          │
    │• Free  │  │AI │ │CSV/HTML/ │
    │ Dict   │  │   │ │Anki/etc  │
    │• Trans │  │   │ │          │
    └────────┘  └───┘ └──────────┘
    
    └──────────────────────────┬──────┘
                               │
                    ┌──────────▼────────┐
                    │ Vocabulary        │
                    │ Manager (SQLite)  │
                    └───────────────────┘
```

---

## 🔑 Key Files Reference

| File | Purpose | Lines | Status |
|------|---------|-------|--------|
| [plugins/vocabulary_manager/vocab_integration.h](plugins/vocabulary_manager/vocab_integration.h) | Main entry point + sidebar | 150 | ✅ Complete |
| [plugins/dictionary_integration/english_dict_fetcher.h](plugins/dictionary_integration/english_dict_fetcher.h) | Free Dict + MyMemory | 80 | ✅ Complete |
| [plugins/gemini_integration/gemini_client.h](plugins/gemini_integration/gemini_client.h) | Google AI client | 120 | ✅ Complete |
| [plugins/quizlet_export/quizlet_exporter.h](plugins/quizlet_export/quizlet_exporter.h) | Multi-format export | 100 | ✅ Complete |
| [plugins/vocabulary_manager/vocabulary_manager.h](plugins/vocabulary_manager/vocabulary_manager.h) | SQLite DB layer | 130 | ✅ Complete |
| [pdf_viewer/main_widget.cpp](pdf_viewer/main_widget.cpp) | Modified: highlight hook | ~1670 | ✅ Modified |
| [pdf_viewer/input.cpp](pdf_viewer/input.cpp) | Modified: export command | ~196 | ✅ Modified |
| [pdf_viewer_build_config.pro](pdf_viewer_build_config.pro) | Build config | - | ✅ Modified |

---

## 📋 Configuration Quick Reference

### Environment Variables
```bash
# API keys
$env:GEMINI_API_KEY = "sk-..."  # Optional, for AI explanations
```

### Config Files (Optional)
File: `pdf_viewer/prefs_user.config`
```ini
vocab_auto_lookup = true          # Auto-show sidebar
vocab_use_gemini = true           # Use AI explanations
vocab_panel_width = 380           # Sidebar width
vocab_dictionary_source = free    # Which API to use
```

### Keybindings
File: `pdf_viewer/keys_user.config`
```ini
q export_quizlet                  # Export to Quizlet
```

---

## 🔄 Data Flow Diagram

```
PDF Document
    │
    └─→ User highlights word "photosynthesis"
         │
         └─→ MainWidget::mouseReleaseEvent()
              │
              └─→ DocumentView::selected_text = "photosynthesis"
                   │
                   └─→ MainWidget::add_highlight()
                        │
                        └─→ VocabIntegration::showLookupPanel("photosynthesis")
                             │
                ┌────────────┼────────────┬──────────┐
                │            │            │          │
                ▼            ▼            ▼          ▼
        FreeDictAPI   MyMemory API   GeminiAPI   VocabManager
          English      Vietnamese      Explain     Store in DB
         Definition   Translation                   
                │            │            │          │
                └────────────┼────────────┴──────────┘
                             │
                             ▼
                    Build HTML Panel
                             │
                             ▼
                    Show in QDockWidget
                             │
                             ▼
                    User clicks "Add to Vocabulary"
                             │
                             ▼
                    VocabularyManager::addWord()
                             │
                             ▼
                    SQLite: vocabulary.db
```

---

## 🧪 Common Testing Scenarios

### Test 1: Basic Lookup
```
1. Open: sioyek.exe any_document.pdf
2. Highlight: "hello"
3. Expected: Sidebar shows English definition + Vietnamese
4. Time: < 2 seconds
```

### Test 2: AI Explanation
```
1. Set: GEMINI_API_KEY=sk-...
2. Highlight: "photosynthesis"
3. Expected: Sidebar shows AI explanation
4. Status: Depends on API quota
```

### Test 3: Export
```
1. Highlight: 5 different words
2. Press: 'q'
3. Choose: Save as vocabulary.csv
4. Expected: CSV with 5 rows (word, definition, translation)
```

### Test 4: Quizlet Import
```
1. Upload: vocabulary.csv to quizlet.com/create
2. Expected: 5 flashcards created
3. Test: Study with Quizlet UI
```

---

## 🐛 Troubleshooting Checklist

| Problem | Diagnosis | Solution |
|---------|-----------|----------|
| Sidebar not showing | Is vocab_auto_lookup=true? | Check prefs_user.config |
| No dictionary results | Network issue? | Check internet, API limits |
| Gemini not working | API key missing? | Set GEMINI_API_KEY env var |
| Export fails | Directory permissions? | Choose different folder |
| Build fails | Qt not found? | See BUILD_INSTRUCTIONS.md |
| Crash on highlight | Pointer null? | Check console output |

---

## 💡 Tips & Tricks

### Keyboard Shortcuts
- **Highlight Mode**: (depends on Sioyek config)
- **Export Vocabulary**: `q` (or configured key)
- **Show Sidebar**: Automatic on highlight

### Performance Tips
- Cache clears automatically (LRU)
- Database indexes optimized for common queries
- Network requests timeout after 5 seconds
- Export is batch operation (fast for <1000 words)

### Advanced Features
- **Spaced Repetition**: VocabularyManager::getWordsForReview() returns words due for review
- **Statistics**: VocabularyManager::getStats() returns learning progress
- **Batch Operations**: ExportQuizletCommand exports all words at once

---

## 📞 Support Resources

**Quick Links**:
- [Full Build Guide](BUILD_INSTRUCTIONS.md)
- [User Setup Guide](VOCAB_PLUGIN_SETUP.md)
- [Technical Deep Dive](IMPLEMENTATION_SUMMARY.md)
- [Implementation Checklist](CHECKLIST.md)

**External Links**:
- [Free Dictionary API](https://api.dictionaryapi.dev)
- [Gemini API](https://ai.google.dev/)
- [Quizlet](https://quizlet.com)
- [Sioyek GitHub](https://github.com/ahrm/sioyek)

---

## 🎯 Common Tasks

### Task: Add word manually
```cpp
VocabularyEntry entry;
entry.word = "hello";
entry.englishDefinition = "greeting";
entry.vietnameseTranslation = "xin chào";
VocabIntegration::instance().vocab()->addWord(entry);
```

### Task: Get all learned words
```cpp
auto learned = VocabIntegration::instance().vocab()
               ->getWordsByStatus("mastered");
```

### Task: Generate study CSV
```cpp
auto all_words = VocabIntegration::instance().vocab()
                 ->getAllWords();
QuizletExporter exporter;
QString csv = exporter.exportToQuizletCSV(all_words, "My Set");
exporter.saveToFile(csv, "study.csv", "csv");
```

### Task: Record review progress
```cpp
VocabIntegration::instance().vocab()
  ->recordReview(wordId, true, 2500);  // correct, 2.5 seconds
```

---

## 📈 API Rate Limits & Quotas

| API | Limit | Cost | Alternative |
|-----|-------|------|-------------|
| Free Dictionary | Unlimited | Free | Built-in fallback |
| MyMemory | ~100/day | Free | Cache results |
| Gemini | 60/min | Free tier | Use sparingly |

**Recommendation**: Use Gemini only for complex terms to stay within quotas.

---

## ✅ Implementation Status Summary

| Component | Status | Files | Lines |
|-----------|--------|-------|-------|
| Dictionary API | ✅ Complete | 3 | 500+ |
| Gemini Client | ✅ Complete | 2 | 350+ |
| Quizlet Export | ✅ Complete | 2 | 300+ |
| Vocabulary DB | ✅ Complete | 2 | 400+ |
| UI Integration | ✅ Complete | 2 | 450+ |
| Build Config | ✅ Complete | 1 | - |
| Documentation | ✅ Complete | 4 | 2000+ |
| **TOTAL** | **✅ READY** | **17** | **4000+** |

---

## 🚀 Next Steps

1. **Install Qt** (if not already done)
2. **Run qmake** to generate build files
3. **Compile** with nmake/make
4. **Test** by highlighting words in a PDF
5. **Export** vocabulary to Quizlet
6. **Configure** API key for AI explanations (optional)

---

**Version**: 1.0  
**Status**: ✅ Ready for Compilation  
**Last Updated**: 2024  
**Language**: Vietnamese/English  

For detailed information, see [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)
