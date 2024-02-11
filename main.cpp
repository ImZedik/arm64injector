//
//  main.cpp
//  arm64injector
//
//  Created by Derek Li on 8/28/23.
//

#include <stdio.h>
#include <vector>
#include "injector.hpp"
#include "terminal.hpp"

int main(int argc, const char * argv[]) {
    if (argc != 3) {
        printf("incorrect number of elements\n");
        return 0;
    }
    const char * pidStr = argv[1];
    pid_t pid;
    sscanf(pidStr, "%d", &pid);
    
    std::string dylibPath = std::string(argv[2]);
    
    bool b = inject(pid, &dylibPath);
    if (b) {
        printf("successfully injected into %d\n", pid);
    }
}
