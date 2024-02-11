//
//  injector.cpp
//  arm64injector
//
//  Created by Derek Li on 8/28/23.
//

#include "injector.hpp"
#include <stdio.h>
#include "frida-core.h"

bool inject(pid_t targetPid, std::string *dylibPath) {
    GError *error = NULL;
    FridaDeviceManager *manager;
    FridaSession *session;
    
    frida_init();
    manager = frida_device_manager_new();
    
    FridaDeviceList *devices;
    devices = frida_device_manager_enumerate_devices_sync(manager, NULL, &error);
    
    if (error != NULL) {
        fprintf(stderr, "frida_device_manager_enumerate_devices_sync failed, error: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    
    FridaDevice *localDevice = NULL;
    gint numDevices = frida_device_list_size(devices);
    
    for (int i = 0; i != numDevices; i++) {
        FridaDevice *device = frida_device_list_get(devices, i);
        
        if (frida_device_get_dtype(device) == FRIDA_DEVICE_TYPE_LOCAL) {
            localDevice = device;
            break;
        }
        
        g_object_unref(device);
    }
    
    if (localDevice == NULL) {
        fprintf(stderr, "finding device type local failed\n");
        return false;
    }
    
    frida_unref(devices);
    
    session = frida_device_attach_sync(localDevice, targetPid, NULL, NULL, &error);
    
    if (error != NULL) {
        fprintf(stderr, "frida_device_attach_sync failed, error: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    
    if (frida_session_is_detached(session)) {
        frida_unref(session);
        fprintf(stderr, "frida session detached prematurely\n");
        return false;
    }
    
    printf("frida attached to process %d\n", targetPid);
    
    std::string jsScript = "var RTLD_NOW = 0x02;\n"
                        "var _dlopen = new NativeFunction(Module.findExportByName(null, \"dlopen\"), 'pointer', ['pointer', 'int']);\n"
    "var path = Memory.allocUtf8String(\"/";
    jsScript.append(*dylibPath);
    jsScript.append("\");\n"
                     "_dlopen(path, RTLD_NOW);\n");
    
    FridaScriptOptions *options;
    options = frida_script_options_new();
    frida_script_options_set_name(options, "loadDylib");
    frida_script_options_set_runtime(options, FRIDA_SCRIPT_RUNTIME_QJS);
    
    FridaScript *script;
    script = frida_session_create_script_sync(session, jsScript.c_str(), options, NULL, &error);
    
    if (error != NULL) {
        fprintf(stderr, "frida_session_create_script_sync failed, error: %s\n", error->message);
        g_error_free(error);
        return false;
    }

    g_clear_object(&options);
    
    frida_script_load_sync(script, NULL, &error);
    
    if (error != NULL) {
        fprintf(stderr, "frida_script_load_sync failed, error: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    
    printf("frida script loaded\n");
    
    frida_script_unload_sync(script, NULL, NULL);
    frida_unref(script);
    frida_session_detach_sync(session, NULL, NULL);
    frida_unref(localDevice);

    frida_device_manager_close_sync(manager, NULL, NULL);
    frida_unref(manager);
    g_print("frida session and device are closed\n");

    return true;
}
