#include "ipc_manager.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

//Unnamed Pipe 

bool IPCManager::createPipe(Pipe& p) {
    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe failed");
        return false;
    }
    p.readFd = fds[0];
    p.writeFd = fds[1];
    return true;
}

bool IPCManager::writeToPipe(const Pipe& p, const std::string& msg) {
    ssize_t n = write(p.writeFd, msg.c_str(), msg.size());
    if (n == -1) {
        perror("write to pipe failed");
        return false;
    }
    return true;
}

std::string IPCManager::readFromPipe(const Pipe& p, size_t maxBytes) {
    std::vector<char> buf(maxBytes + 1);
    ssize_t n = read(p.readFd, buf.data(), maxBytes);
    if (n == -1) {
        perror("read from pipe failed");
        return "";
    }
    buf[n] = '\0';
    return std::string(buf.data());
}

//Named Pipe (FIFO)

bool IPCManager::createFIFO(const std::string& path, mode_t mode) {
    if (mkfifo(path.c_str(), mode) == -1) {
        if (errno == EEXIST) {
            return true; // already exists
        }
        perror("mkfifo failed");
        return false;
    }
    return true;
}

bool IPCManager::writeToFIFO(const std::string& path, const std::string& msg) {
    int fd = open(path.c_str(), O_WRONLY);
    if (fd == -1) {
        perror("open FIFO for write failed");
        return false;
    }
    ssize_t n = write(fd, msg.c_str(), msg.size());
    if (n == -1) {
        perror("write to FIFO failed");
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

std::string IPCManager::readFromFIFO(const std::string& path, size_t maxBytes) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        perror("open FIFO for read failed");
        return "";
    }

    std::vector<char> buf(maxBytes + 1);
    ssize_t n = read(fd, buf.data(), maxBytes);
    if (n == -1) {
        perror("read from FIFO failed");
        close(fd);
        return "";
    }
    buf[n] = '\0';
    close(fd);
    return std::string(buf.data());
}

//Shared Memory

int IPCManager::createSharedMemory(const std::string& name, size_t size) {
    int fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open failed");
        return -1;
    }

    if (ftruncate(fd, size) == -1) {
        perror("ftruncate failed");
        close(fd);
        return -1;
    }

    return fd;
}

void* IPCManager::mapSharedMemory(int fd, size_t size) {
    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap failed");
        return nullptr;
    }
    return addr;
}

bool IPCManager::unlinkSharedMemory(const std::string& name) {
    if (shm_unlink(name.c_str()) == -1) {
        perror("shm_unlink failed");
        return false;
    }
    return true;
}