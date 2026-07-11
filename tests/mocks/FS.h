#ifndef FS_MOCK_H
#define FS_MOCK_H

#include <string>
#include <map>
#include <cstring>
#include "Arduino.h"

extern std::map<std::string, std::string> mock_files;

class File {
public:
    File() : _path(""), _mode(""), _valid(false), _pos(0) {}
    File(const std::string& path, const std::string& mode)
        : _path(path), _mode(mode), _valid(true), _pos(0) {
        if (mode == "w") {
            // Overwrite: truncate
            mock_files[path] = "";
            _pos = 0;
        } else if (mode == "a") {
            // Append: ensure key exists, start writing at end
            if (mock_files.find(path) == mock_files.end()) {
                mock_files[path] = "";
            }
            _pos = mock_files[path].size();
        } else {
            // Read mode: start at beginning
            _pos = 0;
        }
    }

    operator bool() const { return _valid; }

    // --- Positional write: supports seek() ---
    size_t write(const uint8_t *buf, size_t size) {
        if (!_valid) return 0;
        auto& content = mock_files[_path];
        // Extend the backing store if needed to reach write position
        if (_pos + size > content.size()) {
            content.resize(_pos + size, '\0');
        }
        for (size_t i = 0; i < size; i++) {
            content[_pos + i] = (char)buf[i];
        }
        _pos += size;
        return size;
    }

    size_t print(const char* str) {
        if (!_valid) return 0;
        size_t len = std::strlen(str);
        return write(reinterpret_cast<const uint8_t*>(str), len);
    }

    size_t println(const char* str) {
        size_t n = print(str);
        n += print("\n");
        return n;
    }

    size_t print(const String& str) {
        return print(str.c_str());
    }

    size_t println(const String& str) {
        return println(str.c_str());
    }

    int read() {
        if (!_valid || _pos >= mock_files[_path].size()) return -1;
        return (uint8_t)mock_files[_path][_pos++];
    }

    int available() {
        if (!_valid) return 0;
        return (int)(mock_files[_path].size() - _pos);
    }

    // --- Seek support for random-access writes ---
    bool seek(size_t pos) {
        if (!_valid) return false;
        _pos = pos;
        return true;
    }

    size_t position() const { return _pos; }
    size_t size()     const {
        auto it = mock_files.find(_path);
        if (it == mock_files.end()) return 0;
        return it->second.size();
    }

    void close() {
        _valid = false;
    }

private:
    std::string _path;
    std::string _mode;
    bool        _valid;
    size_t      _pos;
};

class FS {
public:
    File open(const char* path, const char* mode = "r") {
        std::string p(path);
        std::string m(mode);
        if (m == "r") {
            if (mock_files.find(p) == mock_files.end()) {
                return File(); // invalid
            }
        }
        return File(p, m);
    }

    File open(const String& path, const char* mode = "r") {
        return open(path.c_str(), mode);
    }

    bool exists(const char* path) {
        return mock_files.find(std::string(path)) != mock_files.end();
    }

    bool exists(const String& path) {
        return exists(path.c_str());
    }

    bool remove(const char* path) {
        std::string p(path);
        if (mock_files.find(p) != mock_files.end()) {
            mock_files.erase(p);
            return true;
        }
        return false;
    }

    bool remove(const String& path) {
        return remove(path.c_str());
    }
};

#endif // FS_MOCK_H
