#ifndef PLATFORM_UTILS_H
#define PLATFORM_UTILS_H
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
#include <string>

inline std::string getResourcePath() {
#ifdef __APPLE__
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  if (mainBundle == NULL) return "assets/";

  CFURLRef appUrlRef = CFBundleCopyBundleURL(mainBundle);
  if (appUrlRef == NULL) return "assets/";

  CFStringRef macPath =
      CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
  if (macPath == NULL) {
    CFRelease(appUrlRef);
    return "assets/";
  }

  char path[1024];
  std::string resourcePath = "assets/";
  if (CFStringGetCString(macPath, path, sizeof(path), kCFStringEncodingUTF8)) {
    resourcePath = std::string(path) + "/Contents/Resources/";
  }

  CFRelease(appUrlRef);
  CFRelease(macPath);

  return resourcePath;
#elif defined(_WIN32)
  return "assets/";
#else
  return "assets/";
#endif
}

#endif
