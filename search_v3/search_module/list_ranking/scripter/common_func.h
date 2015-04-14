#ifndef COMMON_FUNC_H
#define COMMON_FUNC_H
#include <string>
#include <vector>

static bool ParseFormula(std::string& fm, std::vector<std::string>& fields) {
    size_t pos = fm.find('[');
    size_t end = 0;
    while(std::string::npos != pos) {
        end = fm.find(']', pos);
        if( std::string::npos != end) {
            fields.push_back(fm.substr(pos+1, end-pos-1));
            fm.replace(pos, 1, " ");
            fm.replace(end, 1, "_");
            pos = end;
        }
        pos = fm.find('[', pos+1);
    }
    return true;
}

#endif // ~>.!.<~
