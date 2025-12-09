#ifndef IPC_MANAGER_H
#define IPC_MANAGER_H

#include <string>
#include <sys/stat.h>   // Needed for mode_t (permissions for FIFO creation)

/*
 * Pipe struct:
 * Holds the file descriptors used for unnamed pipes.
 *  - readFd:  file descriptor for reading
 *  - writeFd: file descriptor for writing
 */
struct Pipe {
    int readFd;
    int writeFd;
};

/*
 * IPCManager:
 * Provides helper functions for Inter‑Process Communication (IPC),
 * including:
 *   - Unnamed Pipes
 *   - Named Pipes (FIFOs)
 *   - Shared Memory
 *
 * All methods are static, meaning they can be used without creating
 * an instance of IPCManager.
 */
class IPCManager {
public:
    // -------------------------------
    // Unnamed Pipe Methods
    // -------------------------------

    // Creates a standard unnamed pipe (parent-child communication)
    // Fills the Pipe struct with read/write file descriptors.
    static bool createPipe(Pipe& p);

    // Writes a string message to the pipe's write end.
    static bool writeToPipe(const Pipe& p, const std::string& msg);

    // Reads up to 'maxBytes' of data from the pipe's read end.
    // Returns the read string.
    static std::string readFromPipe(const Pipe& p, size_t maxBytes = 1024);


    // -------------------------------
    // Named Pipe (FIFO) Methods
    // -------------------------------

    // Creates a FIFO special file at the given path.
    // 'mode' determines read/write permissions (default: 0666 for rw-rw-rw-)
    static bool createFIFO(const std::string& path, mode_t mode = 0666);

    // Writes 'msg' into the FIFO located at 'path'.
    static bool writeToFIFO(const std::string& path, const std::string& msg);

    // Reads from the FIFO file up to 'maxBytes'.
    static std::string readFromFIFO(const std::string& path, size_t maxBytes = 1024);


    // -------------------------------
    // Shared Memory Methods
    // -------------------------------

    // Creates (or opens if it already exists) a POSIX shared memory object.
    // Returns a file descriptor for the shared memory segment.
    static int createSharedMemory(const std::string& name, size_t size);

    // Maps the shared memory into the process's address space.
    // Returns a pointer to the mapped memory region.
    static void* mapSharedMemory(int fd, size_t size);

    // Unlinks (deletes) the shared memory object so it is removed from the system.
    static bool unlinkSharedMemory(const std::string& name);
};

#endif
