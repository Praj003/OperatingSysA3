#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <limits>

#define SIZE 20
#define HALF (SIZE / 2)

//////////////////////////////// Min val in array function
void find_min(int arr[], int start, int end, int &min_val) {
    min_val = std::numeric_limits<int>::max();
    for (int i = start; i < end; i++) {
        if (arr[i] < min_val) {
            min_val = arr[i];}}
}
//////////////////////////////// Main
int main(int argc, char *argv[]) {
    if (argc > 1 && std::string(argv[1]) == "child") {
        HANDLE read_pipe = (HANDLE) std::stoull(argv[2], nullptr, 10);
        int arr[HALF];
        DWORD bytes_read;
        if (!ReadFile(read_pipe, arr, sizeof(arr), &bytes_read, NULL)) {
            std::cerr << "child pipe failed\n";
            return 1;}
        CloseHandle(read_pipe);
        int min_child;
        find_min(arr, 0, HALF, min_child);
        std::cout << "child id: " << GetCurrentProcessId() << "\n";
        std::cout << "child min #: " << min_child << "\n";
        return min_child; }

///////////////////////  The parent process
    srand(time(0));
    int arr[SIZE];

///////////////////////////////// Filling array
    std::cout << "array: ";
    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand() % 100;
        std::cout << arr[i] << " ";}

    std::cout << std::endl;

//////////////////////////??????? creating "pipe"
    HANDLE read_pipe, write_pipe;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) {
        std::cerr << "pipe failed\n";
        return 1;}

////////////////////////////// HANDLE to string for child process
    std::string read_pipe_str = std::to_string((uintptr_t) read_pipe);

/////////////////////////////////////////// Create child process
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    std::string cmd_line = std::string(argv[0]) + " child " + read_pipe_str;

////////////////////////////// if else for fail / chekc
    if (CreateProcess(NULL, &cmd_line[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);

/////////////////////////////// parent first half min #
        int min_parent;
        find_min(arr, 0, HALF, min_parent);
        std::cout << "parent id: " << GetCurrentProcessId() << "\n";
        std::cout << "parent min: " << min_parent << "\n";

/////////////////////////////// child seconf half min #
        DWORD written;
        WriteFile(write_pipe, &arr[HALF], sizeof(int) * HALF, &written, NULL);
        CloseHandle(write_pipe); // Done writing

///////////////////////////// Waiting for object child
        WaitForSingleObject(pi.hProcess, INFINITE);

////////////////////////////// child min in exit code
        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        CloseHandle(pi.hProcess);
        CloseHandle(read_pipe);
        std::cout << "best min #: " << std::min(min_parent, (int)exit_code) << "\n";}
    else {
        std::cerr << " child procces failed \n";
        return 1;}

    return 0;
}
