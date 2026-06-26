#include <iostream>
#include <cstring>
#include <windows.h>
#include <string>
#include <vector>
#include "shell_ui.h" 
using namespace std;
void createProcess(string x)
{
    char cmdline[1024];
    strncpy(cmdline, x.c_str(), sizeof(cmdline)-1);
    STARTUPINFOA si = { sizeof(si) };    //tells Windows how to set up the new process window   
    PROCESS_INFORMATION pi = {};        //fills it with pi.hProcesss and pi.hthread   
    if(CreateProcessA(
    NULL,        //help windows launch a new program, and the fucntion arguments are the answer to different question
    //asked by the windows 
    cmdline,        
    NULL,          
    NULL,           
    FALSE,          
    0,             
    NULL,           
    NULL,           
    &si,            
    &pi            
) == false)
{
    ShellUI::displayError("AN UNXPECTED ERROR CAME ");
}
else 
{
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}  
}
std::vector<std::string> get_path_directories()  //path resolution
{// 1. Get the raw PATH char* using getenv  2. Split it up    3. Return the collection
    std::vector<std::string> directories;
    char* raw_path = getenv("PATH"); //getting path 
    if (raw_path == nullptr) {
    // If it's missing, just return an empty vector and stop
    return directories; 
    }
    std::string path_str(raw_path); //that vector of path is converted into string  
    //spliting is done here 
    size_t pos = 0;
    size_t found;
    while((found = path_str.find(';', pos)) != string::npos)
    {
    // found is the index of the ';' extract substring from pos to found push into directories  update pos  
    //using find fucntion to find ; which separates 2 paths
        string chunk = path_str.substr(pos, found - pos);
        directories.push_back(chunk);
        pos = found + 1;
    }
    directories.push_back(path_str.substr(pos));
    return directories;
}
string find_in_path(string command) //would return the full path if found , if not just return empty string 
{
        vector<string> dirs = get_path_directories();
        for(int i = 0; i < dirs.size(); i++)
    {
        string full_path = dirs[i] + "\\" + command;
        // check if full_path exists
        // if yes, return full_path
        if(GetFileAttributesA(full_path.c_str()) != INVALID_FILE_ATTRIBUTES)
        return full_path; //will deal with both if written cmd or wirtten cmd.exe
        string full_path_exe = dirs[i] + "\\" + command + ".exe";
        if(GetFileAttributesA(full_path_exe.c_str()) != INVALID_FILE_ATTRIBUTES)
        return full_path_exe;
    }
        return "";
}
vector<string> split_pipe(string input) //splitting commands so that a pipe can execute one command after the other 
{
    vector<string> parts;
    size_t pos = input.find('|'); //commands are separated by | which is used to separate the commands and store in a vector
    if(pos != string::npos)
    {
        parts.push_back(input.substr(0, pos)); //1 part is stored here
        parts.push_back(input.substr(pos + 2)); //other is stored here 
    }
    return parts; //the vector is returned 
} 
void execute_pipe(string cmd1, string cmd2) //creating pipe to connect 2 working environments 
{   //DESCLAIMER: cmd1 helps in re directing output to the writting end of the pipe and cmd2 helps reading the output
    //and take it as input from the reading end of the pipe
    char cmdline1[1024];
    strncpy(cmdline1, cmd1.c_str(), sizeof(cmdline1)-1);
    HANDLE readEnd, writeEnd;
    CreatePipe(&readEnd, &writeEnd, NULL, 65536); //takes input the output ofone enivronment instead of taking 
    //input from the keyboard
    SetHandleInformation(readEnd, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(writeEnd, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    STARTUPINFOA si1 = { sizeof(si1) };
    si1.dwFlags = STARTF_USESTDHANDLES;
    si1.hStdOutput = writeEnd;//instead of displaying output to the screen of first process it redirects output 
    //into the writing end of the pipe 
    si1.hStdInput = GetStdHandle(STD_INPUT_HANDLE); //cmd1 reads from the keyboard normally 
    si1.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    PROCESS_INFORMATION pi1 = {};
    if(CreateProcessA(NULL, cmdline1, NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1) == false)
    {
        ShellUI::displayError("Error running cmd1");
    }
    CloseHandle(writeEnd);
    //cmd2 
    char cmdline2[1024]; //array initialised 
    strncpy(cmdline2, cmd2.c_str(), sizeof(cmdline2)-1);
    STARTUPINFOA si2 = { sizeof(si2) };
    si2.dwFlags = STARTF_USESTDHANDLES;
    si2.hStdInput = readEnd; //it reads the output from the writting end of the pipe
    si2.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si2.hStdError = GetStdHandle(STD_ERROR_HANDLE); //displays output on the screen 
    PROCESS_INFORMATION pi2 = {};
    if(CreateProcessA(NULL, cmdline2, NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2) == false)
        ShellUI::displayError("Error running cmd2");
    WaitForSingleObject(pi1.hProcess, INFINITE);//wait for cmd1 to finish
    WaitForSingleObject(pi2.hProcess, INFINITE);//wait for cmd2 to finish
    CloseHandle(pi1.hProcess);  
    CloseHandle(pi1.hThread); 
    CloseHandle(pi2.hProcess); 
    CloseHandle(pi2.hThread);
    CloseHandle(readEnd); //cleanup the reading end as we are done with it
}
vector<string> tokenize(string input) //helps in separate command and argument
{   //because
    //code needs to make decisions based on the command name and act on the arguments separately.
    vector<string> tokens;
    size_t pos = 0;
    size_t found;
    while((found = input.find(' ', pos)) != string::npos)
    {
        string chunk = input.substr(pos, found - pos);
        if(!chunk.empty()) tokens.push_back(chunk); // skip double spaces
        pos = found + 1;
    }
    tokens.push_back(input.substr(pos)); // last token
    return tokens;
}
    int main ()
{
    vector<string> history;
    string input;
    SetConsoleOutputCP(CP_UTF8);  // For emojis
    ShellUI::displayBanner();      //Banner - ONCE
    SetConsoleCtrlHandler([](DWORD signal) -> BOOL {
    if(signal == CTRL_C_EVENT) return TRUE;
    return FALSE;
    }, TRUE);
    while(true)
    {
        char buffer[256];
        GetCurrentDirectoryA(256, buffer);
        ShellUI::displayPrompt();   
        if(cin.fail()) { cin.clear(); continue; }
        getline(cin, input);
        if(input.empty()) continue;
        vector<string> tokens = tokenize(input);
        history.push_back(input); //added to the history which could be shown in the terminal as per user request 
        if(tokens[0] == "exit")
        {
            break;
        }
        else if(tokens[0] == "cd")//used to change the directory
        {
            string path = tokens[1];
            if(SetCurrentDirectoryA(path.c_str()) == false)
                ShellUI::displayError("Directory not found");
        }
        else if(tokens[0]== "pwd") //used to print current directory
        { //gives the entire path
            char buffer[256];
            GetCurrentDirectoryA(256, buffer);
            cout << buffer << endl;
        }
        else if(tokens[0] == "echo") //used to print on the terminal
        {
            string output = "";
            for(int i = 1; i < tokens.size(); i++)
            output += tokens[i] + " ";
            cout << output << endl;
        }
        else if(tokens[0] == "history")
        {
            for(int i=0 ; i< history.size() ; i++)
            {
                cout << i+1 << ". " << history[i] << endl;
            }
        }
         //if 2 commands are given split pipe is called to split them and execute them
        else if(input.find("|") != string::npos)
        {
            vector<string> parts = split_pipe(input);
            execute_pipe(parts[0], parts[1]);
        }
        else if(input.find(">>") != string::npos)
        {
            size_t pos = input.find(">>");
            string command = input.substr(0, pos - 1);
            string filename = input.substr(pos + 3);
            HANDLE fileHandle = CreateFileA(
            filename.c_str(),
            GENERIC_WRITE,
            0, NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if(fileHandle == INVALID_HANDLE_VALUE)
        {
            ShellUI::displayError("Could not open file");
            continue;
        }
            SetFilePointer(fileHandle, 0, NULL, FILE_END);
            SetHandleInformation(fileHandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            char cmdline[1024];
            strncpy(cmdline, command.c_str(), sizeof(cmdline)-1);
            STARTUPINFOA si1 = { sizeof(si1) };
            si1.dwFlags = STARTF_USESTDHANDLES;
            si1.hStdOutput = fileHandle;
            si1.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            si1.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            PROCESS_INFORMATION pi1 = {};
            if(CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1) == false)
                ShellUI::displayError("Error running command");
            else
            {
                WaitForSingleObject(pi1.hProcess, INFINITE);
                CloseHandle(pi1.hProcess);
                CloseHandle(pi1.hThread);
                CloseHandle(fileHandle);
            }
        }
        else if(input.find(">") != string::npos) //To see wether the input contains ">"
        { //output redirection ... input is taken from the kyeboard and output is given in an output.txt file made 
            //automatically
            size_t pos = input.find(">");
            string command = input.substr(0, pos - 1);
            string filename = input.substr(pos + 2);
            HANDLE fileHandle = CreateFileA( //arguments to create a file
            filename.c_str(),
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if(fileHandle == INVALID_HANDLE_VALUE)
        {
            ShellUI::displayError("Could not open file");
            continue;
        }
        SetHandleInformation(fileHandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        char cmdline[1024];
        strncpy(cmdline, command.c_str(), sizeof(cmdline)-1);
        STARTUPINFOA si1 = { sizeof(si1) };
        si1.dwFlags = STARTF_USESTDHANDLES;
        si1.hStdOutput = fileHandle; //ouput given in external file
        si1.hStdInput = GetStdHandle(STD_INPUT_HANDLE);//input read from the keyboard 
        si1.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        PROCESS_INFORMATION pi1 = {};
        if(CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1) == false)
        ShellUI::displayError("Error running command");
        else
        {
            WaitForSingleObject(pi1.hProcess, INFINITE);
            CloseHandle(pi1.hProcess); //when process finishes.. proceeds to close the file as no data 
            //is incoming
            CloseHandle(pi1.hThread);
            CloseHandle(fileHandle);
        }
    }
        else if(input.find("<") != string::npos) //To see wether the input contains "<"
        {  //reads content from an external file and prints on the screen (output.txt)
            size_t pos = input.find("<");
            string command = input.substr(0, pos - 1);
            string filename = input.substr(pos + 2);
            HANDLE fileHandle = CreateFileA(
            filename.c_str(), //arguments for the windows eg-> which file to read,is it read-only etc
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if(fileHandle == INVALID_HANDLE_VALUE) //if the input becomes invalid file cant be opened 
        {
            ShellUI::displayError("Could not open file");
            continue;
        }
        SetHandleInformation(fileHandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        char cmdline[1024];
        strncpy(cmdline, command.c_str(), sizeof(cmdline)-1);
        STARTUPINFOA si1 = { sizeof(si1) };
        si1.dwFlags = STARTF_USESTDHANDLES;
        si1.hStdInput = fileHandle; //input from external file
        si1.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE); //output shown on the terminal 
        si1.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        PROCESS_INFORMATION pi1 = {};
        if(CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1) == false)
        ShellUI::displayError("Error running command");
        else
        {
            WaitForSingleObject(pi1.hProcess, INFINITE);
            CloseHandle(pi1.hProcess); //after process is done file is closed 
            CloseHandle(pi1.hThread);
            CloseHandle(fileHandle);
        }
        }
        else
        {
            string result = find_in_path(tokens[0]); //if command is not found
            if(result == "")
                ShellUI::displayError("YOUR COMMAND CAN NOT BE FOUND");
            else
                createProcess(result + " " + input.substr(tokens[0].length()));
        }
    }
}