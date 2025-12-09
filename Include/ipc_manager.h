#ifndef IPC_MANAGER_H
#define IPC_MANAGER_H

#include <string>
#include <sys/stat.h> 

struct Pipe {
    int readFd;
    int writeFd;
};

class IPCManager {
public:
    //Unnamed pipe 
    static bool createPipe(Pipe& p);
    static bool writeToPipe(const Pipe& p, const std::string& msg);
    static std::string readFromPipe(const Pipe& p, size_t maxBytes = 1024);

    // Named pipe (FIFO)
    static bool createFIFO(const std::string& path, mode_t mode = 0666);
    static bool writeToFIFO(const std::string& path, const std::string& msg);
    static std::string readFromFIFO(const std::string& path, size_t maxBytes = 1024);

    //Shared memory
    static int createSharedMemory(const std::string& name, size_t size);
    static void* mapSharedMemory(int fd, size_t size);
    static bool unlinkSharedMemory(const std::string& name);
};

#endif