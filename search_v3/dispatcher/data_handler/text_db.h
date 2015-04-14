#ifndef TEXT_DB_H
#define TEXT_DB_H
#include "swstd.h"
#include <vector>
#include <string>
#include <fstream>
#include "Detail.h"


class TextDB {
public:
    TextDB() {}
    ~TextDB() {}
    bool Init(const std::string& db_path);
    bool CoInit();
    int WriteData(const std::vector<char>& text);
    bool ReadData(int index, std::vector<char>& text);
    void Resize(int cursor);
    int GetSize() {
        return *m_cursor_ptr;
    }
private:
    int* m_cursor_ptr;
    CMMapFile m_state_file;
    CDetail m_text_store;
    /// Disallow copy and assign defaultly
    DISALLOW_COPY_AND_ASSIGN(TextDB);
};

#endif // ~>.!.<~ 
