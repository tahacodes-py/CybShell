#pragma once
#include <iostream>
#include <string>
#include <windows.h>
using namespace std;

class ShellUI {
private:
    static constexpr const char* RESET   = "\033[0m";
    static constexpr const char* BLACK   = "\033[30m";
    static constexpr const char* GREEN   = "\033[32m";
    static constexpr const char* BGREEN  = "\033[92m";  // bright green
    static constexpr const char* DGREEN  = "\033[2;32m"; // dim green
    static constexpr const char* RED     = "\033[31m";
    static constexpr const char* BRED    = "\033[91m";
    static constexpr const char* YELLOW  = "\033[33m";
    static constexpr const char* CYAN    = "\033[36m";
    static constexpr const char* WHITE   = "\033[97m";
    static constexpr const char* BOLD    = "\033[1m";
    static constexpr const char* DIM     = "\033[2m";

public:
    static void displayBanner() {
        cout << GREEN;
        cout << R"(
  ██████╗██╗   ██╗██████╗ ███████╗██╗  ██╗███████╗██╗     ██╗
 ██╔════╝╚██╗ ██╔╝██╔══██╗██╔════╝██║  ██║██╔════╝██║     ██║
 ██║      ╚████╔╝ ██████╔╝███████╗███████║█████╗  ██║     ██║
 ██║       ╚██╔╝  ██╔══██╗╚════██║██╔══██║██╔══╝  ██║     ██║
 ╚██████╗   ██║   ██████╔╝███████║██║  ██║███████╗███████╗███████╗
  ╚═════╝   ╚═╝   ╚═════╝ ╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝
)" << RESET;

        cout << DIM << GREEN;
        cout << "  [ Windows Shell  //  Win32 API  //  C++  //  v2.0 ]\n";
        cout << RESET << "\n";

        cout << DGREEN << "  " << string(62, '-') << "\n" << RESET;

        cout << GREEN << "  BUILT-INS" << RESET
             << DIM << "   cd  pwd  echo  history  exit\n" << RESET;

        cout << GREEN << "  OPERATORS" << RESET
             << DIM << "   |  >  >>  <\n" << RESET;

        cout << GREEN << "  PROGRAMS " << RESET
             << DIM << "   any executable in PATH\n" << RESET;

        cout << DGREEN << "  " << string(62, '-') << "\n\n" << RESET;
    }

    static void displayPrompt() {
        char buffer[512];
        GetCurrentDirectoryA(512, buffer);
        string dir(buffer);
        size_t lastSlash = dir.find_last_of("\\");
        string dirName = (lastSlash != string::npos) ? dir.substr(lastSlash + 1) : dir;

        cout << DIM << GREEN << "[ " << RESET
             << BGREEN << BOLD << dirName << RESET
             << DIM << GREEN << " ]" << RESET
             << GREEN << " >> " << RESET;
    }

    static void displayError(const string& msg) {
        cout << BRED << "  ERR  " << RESET
             << RED << msg << RESET << "\n";
    }

    static void displaySuccess(const string& msg) {
        cout << BGREEN << "  OK   " << RESET
             << GREEN << msg << RESET << "\n";
    }

    static void displayInfo(const string& msg) {
        cout << CYAN << "  INF  " << RESET
             << DIM << msg << RESET << "\n";
    }
};