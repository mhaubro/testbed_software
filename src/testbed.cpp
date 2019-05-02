#include "testbed.h"
#include "getmac.h"
#include "execcmd.h"
#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <future>
#include <iostream>
#include <chrono>
#include <thread>

int pid_main = -1;
int pid_acm0 = -1;
int pid_acm1 = -1;


//We assume the mac should only be found once.
char mac[18];

/*Gets dropbox directory*/
std::string getDBDirectory(){
    std::string homedir = getenv("HOME");
    homedir += "/Dropbox";
    return homedir;
}

/*Gets my directory*/
std::string getMyDirectory(){
    std::string homedir = getenv("HOME");
    homedir += "/Dropbox/testbed/" + std::string(mac);
    return homedir;
}

/*Gets the full path to the reset-file*/
std::string resetFilePath(){
    std::string dir = getMyDirectory() + SIGNAL_FOLDER_NAME + SIGNAL_RESET_FILE;
    return dir;
}

/*Gets the full path to the flash-signal-file*/
std::string flashsignalFilePath(){
    std::string dir = getMyDirectory() + SIGNAL_FOLDER_NAME + SIGNAL_FLASH_FILE;
    return dir;
}

std::string flashFileFolder(){
    std::string path = getMyDirectory() + "/flash";
    return path;
}

/*Gets the full path to the flash-file to be flashed to the device*/
std::string flashFilePath(){
    std::string path = flashFileFolder();
    /*Find any file in this folder*/
    return path;
}

std::string logFileFolder(){
    std::string path = getMyDirectory() + "/logs";
    return path;
}

std::string getMyLogFile(){
    std::string path = logFileFolder() + "/log1";
    std::string cmd = "mkdir -p " + path;
    system(cmd.c_str());
    return path;
}

void deleteFile(std::string path){
    std::string command = "rm " + path;
    system(command.c_str());
}

bool folderExists(std::string path){

    struct stat info;

    if(stat( &path[0], &info ) != 0)
        return false;
    else if(info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

bool fileExists(std::string path){

    struct stat info;

    if(stat( &path[0], &info ) != 0)
        return false;
    else if(info.st_mode & S_IFREG)
        return true;
    else
        return false;
}

/*Runs the system command to pipe to output. Is blocking until the thread is terminated somehow or grabserial is terminated*/
void outputSerial(std::string serial_device){
    std::string command = "sudo grabserial -v -d " + serial_device + " -b 115200 -w 8 -p N -s 1 -t > " + getMyLogFile(); 
}

/*Checks if we have dropbox. Creates other necessary directories if needed*/
bool directoryCheck(){
    if (!folderExists(getDBDirectory().c_str())){
        return false;
    }

    std::string mydir = getMyDirectory();

    std::string paths_needed[2 + LOGCOUNT];

    paths_needed[0] = logFileFolder();
    paths_needed[1] = flashFileFolder();
    /*Create log folders*/
    for (int i = 0; i < LOGCOUNT; i++){
        paths_needed[2 + i] = logFileFolder() + "/log" + std::to_string(i);
    }

    for (int i = 0; i < (2 + LOGCOUNT); i++){
        std::string command = "mkdir -p " + paths_needed[i];
        system(command.c_str());
    }

    return true;
}

/*Restes logs, moves log1 to log2, log2 to log3 etc*/
void resetLogs(){
    std::string deleteLastFolder = "rm -rf " + logFileFolder() + "/log" + std::to_string(LOGCOUNT - 1);
    system(deleteLastFolder.c_str());

    for (int i = LOGCOUNT - 2; i >= 0; i--){
        /*Rename folder log_i to log_i+1*/
        std::string cmd = "mv " + logFileFolder() + "/log" + std::to_string(i) + " " + logFileFolder() + "/log" + std::to_string(i + 1);
        system(cmd.c_str());
    }
    std::string deleteContentOfFirstFolder = "rm -rf " + logFileFolder() + "/log" + std::to_string(1) + "/*";
    system(deleteLastFolder.c_str());
}

/*Checks if we have any device connected at all - will only run at startup*/
bool checkForDevice(){

    /*Old idea: Check dmesg*/
    /*
    std::string serial_devices = exec("dmesg | grep tty\n");
    */
    /*Very hardcoded way of checking for devices, but as it is always on one of the two on both ubuntu server, antergos and manjaro */

    /*
    if ((serial_devices.find("ttyACM0") != std::string::npos) || (serial_devices.find("ttyACM1") != std::string::npos)) {
        return true;
    }
    */

    /* New Idea: execute grabserial, see what happens */

    return false;
}

void startExperiment(){

}

void flashDevice(){

}

bool flashFlag(){
    return true;

}

bool resetFlag(){
    return true;

}

void programLoop(){

    while(true){
        if (resetFlag()){

        } else if (flashFlag()){

        }


        std::cout << "Hi\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main(){
    pid_main = getpid();
    bool macSucces = getMacAddress(mac);

    if (!macSucces){
        /*Maybe do something intelligent here, fake a pseudo unique id, find a uid a different way*/
    }
    if (!directoryCheck()){
        /*We stop the program*/
        std::cout << "No dropbox\n";
    }

/*    std::cout << exec("dmesg | grep tty\n");
    std::cout << exec("dmesg | grep tty\n");

    programLoop();*/

    system("sudo grabserial -v -d \"/dev/ttyACM0\" -b 115200 -w 8 -p N -s 1 -t > log.txt");

    return 0;
}

