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
            mock_files[path] = "";
        }
    }
    
    operator bool() const { return _valid; }
    
    size_t write(const uint8_t *buf, size_t size) {
        if (!_valid) return 0;
        mock_files[_path].append((const char*)buf, size);
        return size;
    }
    
    size_t print(const char* str) {
        if (!_valid) return 0;
        mock_files[_path].append(str);
        return std::strlen(str);
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
        return mock_files[_path].size() - _pos;
    }
    
    void close() {
        _valid = false;
    }

private:
    std::string _path;
    std::string _mode;
    bool _valid;
    size_t _pos;
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
