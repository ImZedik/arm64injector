//
//  injector.hpp
//  arm64injector
//
//  Created by Derek  on 8/28/23.
//

#ifndef injector_hpp
#define injector_hpp

#include <libproc.h>
#include <string>

bool inject(pid_t pid, std::string *dylibPath);

#endif /* injector_hpp */
