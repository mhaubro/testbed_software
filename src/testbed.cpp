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

using namespace std;

//We assume the mac should only be found once.
char mac[18];

/*Gets my directory*/
string getMyDirectory(){
    string homedir = getenv("HOME");
    homedir += "/testbed/" + string(mac);
    return homedir;
}

/*Gets the full path to the reset-file*/
string resetFilePath(){
    string dir = getMyDirectory() + "/signals" + SIGNAL_RESET_FILE;
    return dir;
}

string flashFileFolder(){
    string path = getMyDirectory() + "/flash";
    return path;
}
string signalFileFolder(){
    string path = getMyDirectory() + "/signals";
    return path;
}

string flashSignalPath(){
    string path = signalFileFolder() + SIGNAL_FLASH_FILE;
    /*Find any file in this folder*/
    return path;
}

string logFileFolder(){
    string path = getMyDirectory() + "/logs";
    return path;
}

/*We upload liveness, so it's placed in logs*/
string liveSignalPath(){
    string path = logFileFolder() + SIGNAL_LIVE_FILE;
    /*Find any file in this folder*/
    return path;
}



string GetMyLogFolder(){
    string path = logFileFolder() + "/log0";
    string cmd = "mkdir -p " + path;
    system(cmd.c_str());
    return path;
}

void deleteFile(string path){
    string command = "rm " + path;
    system(command.c_str());
}

void rcloneCommand(string cmd){
    system(string("rclone " + cmd).c_str());
}

void deleteRemote(string path){
    rcloneCommand(string("delete ") + RCLONE_REMOTE + REMOTEROOT + "/" + string(mac) + path);
}


/*Gets array with grabserial process ids*/
vector<int> getGrabSerialProcessArray(){
    /* Grab PIDs of grabserial processes */
    string grabserial_processes = exec("pgrep grabserial");

    vector<int> processes;
    istringstream ss(grabserial_processes);

    string line;

    while(getline(ss, line)) {
        processes.push_back(stoi(line));
	}
    return processes;
}


void terminateGrabSerial(){

    vector<int> processes = getGrabSerialProcessArray();

    /*Do the killing to ensure we start on a clean slate*/
    for (int i = 0; i < processes.size(); i++){
        system(string("kill " + processes[i]).c_str());
    }
}

/*Start grabserial*/
void startGrabSerial(){
    cout << "start_grab" << endl;
    system("pkill grabserial");
    system(string("grabserial -v -d \"/dev/ttyACM0\" -b 115200 -w 8 -p N -s 1 -t -o " + GetMyLogFolder() + "/logACM0.txt"  + " &").c_str());
    system(string("grabserial -v -d \"/dev/ttyACM1\" -b 115200 -w 8 -p N -s 1 -t -o " + GetMyLogFolder() + "/logACM1.txt"  + " &").c_str());
}

/*Terminate all instances and start new ones*/
void resetGrabSerial(){
    terminateGrabSerial();
    startGrabSerial();    
}



bool folderExists(string path){

    struct stat info;

    if(stat( &path[0], &info ) != 0)
        return false;
    else if(info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

bool fileExists(string path){

    struct stat info;

    if(stat( &path[0], &info ) != 0)
        return false;
    else if(info.st_mode & S_IFREG)
        return true;
    else
        return false;
}

/*Runs the system command to pipe to output. Is blocking until the thread is terminated somehow or grabserial is terminated*/
void outputSerial(string serial_device){
    string command = "grabserial -v -d " + serial_device + " -b 115200 -w 8 -p N -s 1 -t > " + GetMyLogFolder(); 
}

void uploadData(){
    cout << "Uploading\n";
    string localpath = getMyDirectory() + string("/logs");
    string remotepath = REMOTEROOT + "/" + string(mac) + "/logs";
    system(string("rclone copy " + localpath + " DTUHPC:" + remotepath + " --create-empty-src-dirs").c_str());
    /*Ensuring that a signals folder will be online, as well as sending the liveness signal*/
    localpath = liveSignalPath();
    remotepath = REMOTEROOT + "/" + string(mac) + "/logs";
    system(string("touch " + liveSignalPath()).c_str());
    system(string("rclone copy " + localpath + " DTUHPC:" + remotepath + " --create-empty-src-dirs").c_str());
}


/*Checks if we have dropbox. Creates other necessary directories if needed*/
bool directoryCheck(){
    string mydir = getMyDirectory();

    string paths_needed[3 + LOGCOUNT];

    paths_needed[0] = logFileFolder();
    paths_needed[1] = flashFileFolder();
    paths_needed[2] = signalFileFolder();
    /*Create log folders*/
    for (int i = 0; i < LOGCOUNT; i++){
        paths_needed[3 + i] = logFileFolder() + "/log" + to_string(i);
    }

    for (int i = 0; i < (3 + LOGCOUNT); i++){
        string command = "mkdir -p " + paths_needed[i];
        system(command.c_str());
    }

    /*Ensure that we are very much online*/
    string localpath = getMyDirectory();
    string remotepath = REMOTEROOT + "/" + string(mac);
    system(string("rclone copy " + localpath + " DTUHPC:" + remotepath + " --create-empty-src-dirs").c_str());

    return true;
}

/*Restes logs, moves log1 to log2, log2 to log3 etc*/
void resetLogs(){
    string deleteLastFolder = "rm -rf " + logFileFolder() + "/log" + to_string(LOGCOUNT - 1);
    system(deleteLastFolder.c_str());

    for (int i = LOGCOUNT - 2; i >= 0; i--){
        /*Rename folder log_i to log_i+1*/
        string cmd = "mv " + logFileFolder() + "/log" + to_string(i) + " " + logFileFolder() + "/log" + to_string(i + 1);
        system(cmd.c_str());
    }
    string deleteContentOfFirstFolder = "rm -rf " + logFileFolder() + "/log" + to_string(1) + "/*";
    system(deleteContentOfFirstFolder.c_str());
}

bool checkGrabSerialAlive(){
    /*If we have less than two processes running it's no good*/
    return (getGrabSerialProcessArray().size() == 2);
}




/* Something better might be nice */
bool checkForDevice(){
    if(checkGrabSerialAlive()){
        return true;
    } 
    startGrabSerial();
    /*Make sure the process have started and terminated if there is no serial device */
    this_thread::sleep_for(chrono::seconds(1));

    /*If we're connected it's good, if not, there is no device*/
    return checkGrabSerialAlive();
}

void resetMCU(){
    system(string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"init reset run\"").c_str());
}

void flashMCU(){
    /*This implies trying to flash all files. SO ONLY LEAVE 1 FILE IN THE DIRECTORY */
    for (const auto & entry : filesystem::directory_iterator(flashFileFolder())){
        system(string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"program \"" + string(entry.path()) + " verify reset exit\"").c_str());
        deleteFile(entry.path());
        //deleteRemote(entry.path());
    }

}

bool flashFlag(){

    if (fileExists(flashSignalPath())){
        deleteFile(flashSignalPath());
        deleteRemote(std::string("/signals") + SIGNAL_FLASH_FILE);
        /*Delete file*/
        return true;
    }
    return false;
}

bool resetFlag(){
    if (fileExists(resetFilePath())){
        deleteFile(resetFilePath());
        deleteRemote(std::string("/signals") + SIGNAL_RESET_FILE);

        return true;
    }
    return false;
}


/*flash and signals*/

void downloadData(){
    cout << "Downloading\n";
    string localpath = getMyDirectory() + string("/signals");
    string remotepath = REMOTEROOT + "/" + string(mac) + string("/signals");
    system(string("rclone copy DTUHPC:" + remotepath + " " + localpath).c_str());

    localpath = getMyDirectory() + string("/flash");
    remotepath = string(REMOTEROOT) + "/" + string(mac) + string("/flash");
    system(string("rclone copy DTUHPC:" + remotepath + " " + localpath).c_str());
}


void programLoop(){

    while(true){

        while (!checkForDevice()){
            /* We wait until we have a device */
            this_thread::sleep_for(chrono::seconds(15));
        }
        uploadData();
        if (resetFlag()){
            terminateGrabSerial();
            resetLogs();
            startGrabSerial();
            resetMCU();
        } 
        if (flashFlag()){
            terminateGrabSerial();
            resetLogs();
            flashMCU();
            startGrabSerial();
            resetMCU();
        }

        downloadData();
        this_thread::sleep_for(chrono::seconds(10));
        //resetGrabSerial();
    }
}








int main(){
    bool macSucces = getMacAddress(mac);

    directoryCheck();
    //uploadData(); /*We'll just show we're online*/
    programLoop();

    //cout <<  getMyDirectory() << endl;

    return 0;
}

