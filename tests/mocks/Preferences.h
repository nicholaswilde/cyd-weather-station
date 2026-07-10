#ifndef PREFERENCES_MOCK_H
#define PREFERENCES_MOCK_H

#include <string>
#include <map>
#include <sstream>

class Preferences {
private:
    bool _opened = false;
    bool _readOnly = false;

    static std::map<std::string, std::string>& getStorage() {
        static std::map<std::string, std::string> storage;
        return storage;
    }

public:
    bool begin(const char * name, bool readOnly = false) {
        _opened = true;
        _readOnly = readOnly;
        return true;
    }

    void end() {
        _opened = false;
    }

    bool clear() {
        if (_readOnly || !_opened) return false;
        getStorage().clear();
        return true;
    }

    bool remove(const char * key) {
        if (_readOnly || !_opened) return false;
        return getStorage().erase(key) > 0;
    }

    size_t putBool(const char* key, bool value) {
        if (_readOnly || !_opened) return 0;
        getStorage()[key] = value ? "1" : "0";
        return 1;
    }

    bool getBool(const char* key, bool defaultValue = false) {
        if (!_opened) return defaultValue;
        auto& storage = getStorage();
        auto it = storage.find(key);
        if (it == storage.end()) return defaultValue;
        return it->second == "1";
    }

    size_t putInt(const char* key, int value) {
        if (_readOnly || !_opened) return 0;
        std::stringstream ss;
        ss << value;
        getStorage()[key] = ss.str();
        return sizeof(int);
    }

    int getInt(const char* key, int defaultValue = 0) {
        if (!_opened) return defaultValue;
        auto& storage = getStorage();
        auto it = storage.find(key);
        if (it == storage.end()) return defaultValue;
        std::stringstream ss(it->second);
        int val;
        ss >> val;
        return val;
    }
};

#endif // PREFERENCES_MOCK_H
