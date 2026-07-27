#include <iostream>
int main() {
    int point_count = 24;
    int major_cnt = 7;
    for(int i=0; i<major_cnt; i++) {
        int val = (i * (point_count - 1)) / (major_cnt - 1);
        std::cout << "Tick " << i << " value: " << val << " (val%4=" << val%4 << ")\n";
    }
}
