#include "testbed.h"
#include "getmac.h"
#include "execcmd.h"
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <thread>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <sstream>
#include <filesystem>



//We assume the mac should only be found once.
char mac[18];

/*Gets my directory*/
std::string getMyDirectory(){
    std::string homedir = getenv("HOME");
    homedir += "/testbed/" + std::string(mac);
    return homedir;
}

/*Gets the full path to the reset-file*/
std::string resetFilePath(){
    std::string dir = getMyDirectory() + SIGNAL_RESET_FILE;
    return dir;
}

/*Gets the full path to the flash-signal-file*/
std::string flashsignalFilePath(){
    std::string dir = getMyDirectory() + SIGNAL_FLASH_FILE;
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

std::string GetMyLogFolder(){
    std::string path = logFileFolder() + "/log1";
    std::string cmd = "mkdir -p " + path;
    system(cmd.c_str());
    return path;
}

void deleteFile(std::string path){
    std::string command = "rm " + path;
    system(command.c_str());
}

/*Gets array with grabserial process ids*/
std::vector<int> getGrabSerialProcessArray(){
    /* Grab PIDs of grabserial processes */
    std::string grabserial_processes = exec("pgrep grabserial");

    std::vector<int> processes;
    std::istringstream ss(grabserial_processes);

    std::string line;

    while(std::getline(ss, line)) {
        processes.push_back(std::stoi(line));
	}
    return processes;
}


void terminateGrabSerial(){

    std::vector<int> processes = getGrabSerialProcessArray();

    /*Do the killing to ensure we start on a clean slate*/
    for (int i = 0; i < processes.size(); i++){
        system(std::string("kill " + processes[i]).c_str());
    }
}

/*Start grabserial*/
void startGrabSerial(){
    system(std::string("sudo grabserial -v -d \"/dev/ttyACM0\" -b 115200 -w 8 -p N -s 1 -t -o " + GetMyLogFolder() + "/logACM0.txt"  + " &").c_str());
    system(std::string("sudo grabserial -v -d \"/dev/ttyACM1\" -b 115200 -w 8 -p N -s 1 -t -o " + GetMyLogFolder() + "/logACM1.txt"  + " &").c_str());
}

/*Terminate all instances and start new ones*/
void resetGrabSerial(){
    terminateGrabSerial();
    startGrabSerial();    
}

void rcloneCommand(std::string cmd){
    system(std::string("rclone " + cmd));
}

void rcloneUploadToExternal(){

}

void rcloneDownloadFromExternal(){
    
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
    std::string command = "sudo grabserial -v -d " + serial_device + " -b 115200 -w 8 -p N -s 1 -t > " + GetMyLogFolder(); 
}

/*Checks if we have dropbox. Creates other necessary directories if needed*/
bool directoryCheck(){
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

bool checkGrabSerialAlive(){
    /*If we have less than two processes running it's no good*/
    return (getGrabSerialProcessArray().size() != 2);
}




/* Something better might be nice */
bool checkForDevice(){
    if(checkGrabSerialAlive()){
        return true;
    } 
    startGrabSerial();
    /*Make sure the process have started and terminated if there is no serial device */
    std::this_thread::sleep_for(std::chrono::seconds(1));

    /*If we're connected it's good, if not, there is no device*/
    return checkGrabSerialAlive();
}

void resetMCU(){
    system(std::string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"init reset run\"").c_str());
}

void flashMCU(){
    std::string path = std::string(getMyDirectory() + "/flash");

    /*This implies trying to flash all files. SO ONLY LEAVE 1 FILE IN THE DIRECTORY */
    for (const auto & entry : std::filesystem::directory_iterator(path)){
        system(std::string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"program " + entry.path() + " verify reset exit\"").c_str())
    }    
}



bool flashFlag(){

    if (fileExists(flashFilePath()){
        deleteFile(flashFilePath());
        system("")
        /*Delete file*/
        return true;
    }
    return false;
}

bool resetFlag(){
    return true;

}

void uploadData(){
    std::string localpath = getMyDirectory();
    std::string remotepath = std::string(REMOTEROOT) + "/" + std::string(mac);
    system(std::string("rclone copy DTUHPC:" + localpath + " " + remotepath).c_str());
}

/*flash and signals*/
void downloadData(){
    std::string localpath = getMyDirectory();
    std::string remotepath = std::string(REMOTEROOT) + "/" + std::string(mac);
    system(std::string("rclone copy DTUHPC:" + remotepath + " " + localpath).c_str());
}

void programLoop(){

    while(true){

        while (!checkForDevice()){
            /* We wait until we have a device */
            std::this_thread::sleep_for(std::chrono::seconds(15));
        }
        downloadData();
        if (resetFlag()){
            terminateGrabSerial();
            resetLogs();
            startGrabSerial();
            resetMCU();
        } else if (flashFlag()){
            terminateGrabSerial();
            resetLogs();
            flashMCU();
            startGrabSerial();
            resetMCU();
        }

        uploadData();

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}








int main(){

    bool macSucces = getMacAddress(mac);

    directoryCheck();

    //uploadData(); /*We'll just show we're online*/
    flashMCU();
    //programLoop();

    //std::cout <<  getMyDirectory() << std::endl;

    return 0;
}

