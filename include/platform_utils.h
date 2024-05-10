#ifndef COREFOUNDATIONUTIL_H 
#define COREFOUNDATIONUTIL_H
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
#include <string>

std::string getResourcePath() {
  #ifdef __APPLE__
    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    std::string path(pathPtr);
    std::string resourcePath = path + "/Contents/Resources/";

    CFRelease(appUrlRef);
    CFRelease(macPath);

    return resourcePath;
  #elif defined(_WIN32)
    // TODO: Windows resources
    return "";
  #elif defined(__LINUX__)
    // TODO: Linux resources
    return "";
  #endif
}

#endif