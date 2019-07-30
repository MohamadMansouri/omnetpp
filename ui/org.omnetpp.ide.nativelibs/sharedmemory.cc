//=========================================================================
//  SHAREDMEMORY.CC - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2019 Andras Varga
  Copyright (C) 2006-2019 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <iostream>

#if defined(__linux__)
  #include <malloc.h>
  #include <sys/sysinfo.h>
#elif defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #define NOGDI
  #include <windows.h>
#elif defined(__APPLE__)
  #include <sys/types.h>
  #include <sys/sysctl.h>
  #include <mach/mach.h>
#endif

#if !defined(_WIN32)
  #include <unistd.h>
  #include <errno.h>
  #include <sys/mman.h>
  #include <sys/stat.h>        /* For mode constants */
  #include <fcntl.h>           /* For O_* constants */
#endif

#include <jni.h>

extern "C" {

#include <string.h>

// utility function, not called from Java
jint throwRuntimeException(JNIEnv *env, const std::string& message)
{
    jclass exClass = env->FindClass("java/lang/RuntimeException");
    return env->ThrowNew(exClass, message.c_str());
}

JNIEXPORT void JNICALL Java_org_omnetpp_scave_engine_ScaveEngineJNI_createSharedMemory(JNIEnv* env, jobject clazz, jstring name, jlong size)
{
    const char *nameChars = env->GetStringUTFChars(name, nullptr);
    char nameStr[2048];
    strncpy(nameStr, nameChars, 2048);
    env->ReleaseStringUTFChars(name, nameChars);

#if defined(_WIN32)

    HANDLE hMapFile = CreateFileMapping(
      INVALID_HANDLE_VALUE,    // use paging file
      NULL,                    // default security
      PAGE_READWRITE,          // read/write access
      0,                       // maximum object size (high-order DWORD)
      size,                    // maximum object size (low-order DWORD)
      nameStr);                // name of mapping object

#else // POSIX (linux, mac)

    int fd = shm_open(nameStr, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        throwRuntimeException(env, std::string("Error opening SHM file descriptor for '") + nameStr + "': " + strerror(errno));
        return;
    }

    if (ftruncate(fd, size) == -1) {
        throwRuntimeException(env, std::string("Error setting SHM file descriptor to size ") + std::to_string(size) + " for '" + nameStr + "': " + strerror(errno));
        return;
    }

    if (close(fd) == -1) {
        throwRuntimeException(env, std::string("Error closing SHM file descriptor for '") + nameStr + "': " + strerror(errno));
        return;
    }

#endif
}

JNIEXPORT jobject JNICALL Java_org_omnetpp_scave_engine_ScaveEngineJNI_mapSharedMemory(JNIEnv* env, jobject clazz, jstring name, jlong size)
{
    const char *nameChars = env->GetStringUTFChars(name, nullptr);
    char nameStr[2048];
    strncpy(nameStr, nameChars, 2048);
    env->ReleaseStringUTFChars(name, nameChars);

    // must be set by the platform specific fragments
    void *buffer = nullptr;

#if defined(_WIN32)

    // this did NOT work: HANDLE opened = OpenFileMapping(PAGE_READWRITE, FALSE, nameStr);

    HANDLE hMapFile = CreateFileMapping(
      INVALID_HANDLE_VALUE,    // use paging file
      NULL,                    // default security
      PAGE_READWRITE,          // read/write access
      0,                       // maximum object size (high-order DWORD)
      size,                       // maximum object size (low-order DWORD) - ignored, but must not be 0
      nameStr);                // name of mapping object

    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        throwRuntimeException(env, std::string("Trying to map a nonexistent shared memory mapping ('") + nameStr + "'), so it was created instead... Expect zeroes!");
        return 0;
    }

    if (hMapFile == NULL) {
        char err[2048];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 2048, NULL);
        throwRuntimeException(env, std::string("Error opening shared memory file mapping '") + nameStr + "': " + err);
    }

    buffer = MapViewOfFile(
      hMapFile,   // handle to map object
      FILE_MAP_ALL_ACCESS, // read/write permission
      0, // no offset (high)
      0, // no offset (low)
      0); // no size, map the whole object

    if (buffer == NULL) {
        char err[2048];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 2048, NULL);
        throwRuntimeException(env, std::string("Error mapping view of file '") + nameStr + "': " + err);
    }

#else // POSIX (linux, mac)

    int fd = shm_open(nameStr, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        throwRuntimeException(env, std::string("Error opening SHM file descriptor '") + nameStr + "': " + strerror(errno));
        return 0;
    }

    buffer = mmap(0, size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        throwRuntimeException(env, std::string("Error mmap-ing SHM file descriptor '") + nameStr + "': " + strerror(errno));
        return 0;
    }

    if (close(fd) == -1) {
        throwRuntimeException(env, std::string("Error closing SHM file descriptor '") + nameStr + "': " + strerror(errno));
        return 0;
    }

#endif

    jobject directBuffer = env->NewDirectByteBuffer(buffer, size);
    return directBuffer;
}

JNIEXPORT void JNICALL Java_org_omnetpp_scave_engine_ScaveEngineJNI_unmapSharedMemory(JNIEnv* env, jobject clazz, jobject directBuffer)
{
    void *buffer = env->GetDirectBufferAddress(directBuffer);
    jlong capacity = env->GetDirectBufferCapacity(directBuffer);

#if defined(_WIN32)
    if (UnmapViewOfFile(buffer) == 0) {
        char err[2048];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 2048, NULL);
        throwRuntimeException(env, std::string("Error unmapping file view of size ") + std::to_string(capacity) + ": " + err);
    }
#else // POSIX (linux, mac)
    if (munmap(buffer, capacity) == -1)
        throwRuntimeException(env, std::string("Error unmapping SHM buffer of size ") + std::to_string(capacity) + ": " + strerror(errno));
#endif
}

JNIEXPORT void JNICALL Java_org_omnetpp_scave_engine_ScaveEngineJNI_removeSharedMemory(JNIEnv* env, jobject clazz, jstring name)
{
    const char *nameChars = env->GetStringUTFChars(name, nullptr);
    char nameStr[2048];
    strncpy(nameStr, nameChars, 2048);
    env->ReleaseStringUTFChars(name, nameChars);

#if defined(_WIN32)

    HANDLE hMapFile = CreateFileMapping(
      INVALID_HANDLE_VALUE,    // use paging file
      NULL,                    // default security
      PAGE_READWRITE,          // read/write access
      0,                       // maximum object size (high-order DWORD)
      1,                       // maximum object size (low-order DWORD) - ignored, but must not be 0
      nameStr);                // name of mapping object

    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        char err[2048];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 2048, NULL);
        throwRuntimeException(env, std::string("Trying to remove a nonexistent mapping '") + nameStr + "': " + err);
        return;
    }

    if (CloseHandle(hMapFile) == 0) {
        char err[2048];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 2048, NULL);
        throwRuntimeException(env, std::string("Error closing file mapping handle '") + nameStr + "': " + err);
    }

#else // POSIX (linux, mac)
    if (shm_unlink(nameStr) == -1)
        throwRuntimeException(env, std::string("Error unlinking SHM object '") + nameStr + "': " + strerror(errno));
#endif
}

} // extern "C"
