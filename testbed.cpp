#include "testbed.h"
#include "getmac.h"
#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <thread>


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

/*Gets the full path to the flash-file to be flashed to the device*/
std::string flashFilePath(){
    std::string path = getMyDirectory() + "/flash";

}

void moveLogs(){

}

bool checkDevice(){
    
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

/*Checks if we have dropbox. Creates other necessary directories if needed*/
bool directoryCheck(){
    if (!folderExists(getDBDirectory().c_str())){
        return false;
    }

    std::string mydir = getMyDirectory();

    std::string paths_needed[6];

    paths_needed[0] = mydir + "/logs";
    paths_needed[1] = mydir + "/flash";

    paths_needed[2] = paths_needed[0] + "/log1";
    paths_needed[3] = paths_needed[0] + "/log2";
    paths_needed[4] = paths_needed[0] + "/log3";

/*Resetting and flashing will be done in root folder of this device*/
//    paths_needed[5] = homedir + "/Dropbox/testbed/" + std::string(mac) + "/reset";

    for (int i = 0; i < 6; i++){
        std::string command = "mkdir -p " + paths_needed[i];
        system(command.c_str());
    }

    return true;
}

void resetLogs(){

}

void startExperiment(){

}

void flashDevice(){

}

bool flashFlag(){

}

bool resetFlag(){

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

    bool macSucces = getMacAddress(mac);

    if (!macSucces){
        /*Maybe do something intelligent here, fake a pseudo unique id, find a uid a different way*/
    }
    if (!directoryCheck()){
        /*We stop the program*/
        std::cout << "No dropbox\n";
    }

    programLoop();

    return 0;
}

