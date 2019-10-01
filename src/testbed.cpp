#include "testbed.h"
#include "getmac.h"
#include "execcmd.h"
#include "auxiliary.h"
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
#include <chrono>
#include <iomanip>
#include <ctime>

using namespace std;

//We assume the mac should only be found once. Also, this is the pure mac, without any slashes.
char mac[18];
int currentLog;

string localSigFromRpiFolder(){
    string path = getMyDirectory() + "/SigFromRpi";
    return path;
}
string localSigToRpiFolder(){
    string path = getMyDirectory() + "/SigToRpi";
    return path;
}
string localOutputUploadFolder(){
    string path = getMyDirectory() + "/output";
    return path;
}
string localOutputFolder(){
    return localOutputUploadFolder();
}

string localFlashFileFolder(){
    string path = localSigToRpiFolder() + FLASHFILEFOLDER;
    return path;
}
string localGoFlagPath(){
    string path = localSigToRpiFolder() + SIGNAL_GO_FILE;
    return path;
}

string localNewExperimentFlagPath(){
    string path = localSigToRpiFolder() + SIGNAL_NEWEXPERIMENT_FILE;
    return path;
}

string myremoteSignalsFromRpiFolder(){
    string path = remoteSignalsFromRpiFolder() + "/" + string(mac);
    return path;
}
string myremoteSignalsToRpiFolder(){
    string path = remoteSignalsToRpiFolder() + "/" + string(mac);
    return path;
}
string myRemoteOutputFolder(){
    string path = remoteOutputFolder() + "/" + string(mac);
    return path;
}

string myRemoteNewExperimentFlagPath(){
    string path = myremoteSignalsToRpiFolder() + SIGNAL_NEWEXPERIMENT_FILE;
    return path;
}
string myRemoteGoFlagPath(){
    string path = myremoteSignalsToRpiFolder() + SIGNAL_GO_FILE;
    return path;
}
string myRemoteLiveSignalPath(){
    string path = myremoteSignalsFromRpiFolder() + SIGNAL_LIVE_FILE;
    return path;
}
string myRemoteStopSignalPath(){
    string path = myremoteSignalsFromRpiFolder() + SIGNAL_STOPPED_FILE;
    return path;    
}
string myremoteFlashFileFolder(){
    string path = myremoteSignalsToRpiFolder() + FLASHFILEFOLDER;
    return path;    
}
/*If larger than 5mb, we just stop */
bool checkLogSizes(){
    if (filesize(localOutputFolder() + "/logacm0.txt") > 1000000){
        return true;
    } else {
        return true;
    }
}

/*If the machine is rebooted or this program restarted, we check if there is something already, to not overwrite old data*/
void checkForExistingLogs(){
/*Not implemented yet*/
}

void AppendToPiLog(string data){
    string piLogFile = localSigFromRpiFolder() + "/pilog.txt";

    /*Get time to print */
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");

    string time = oss.str() + ": ";
    system(string("echo \"" + time + data + "\" >> " + piLogFile).c_str());
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
    AppendToPiLog(string("start_grab"));
    system("pkill grabserial");
    system(string("grabserial -v -d \"/dev/ttyACM0\" -b 115200 -w 8 -p N -s 1 -t --systime -o " + localOutputFolder() + "/logacm0.txt"  + " &").c_str());
    //system(string("grabserial -v -d \"/dev/ttyACM1\" -b 115200 -w 8 -p N -s 1 -t --systime > " + localOutputFolder() + "/logacm1.txt"  + " -S -T &").c_str());
}

/*Terminate all instances and start new ones*/
void resetGrabSerial(){
    startGrabSerial();    
}

void publishMe(){
    /*Sanity checks*/
    rcloneCommand("mkdir " + REMOTEROOT);
    rcloneCommand("mkdir " + remoteOutputFolder());
    rcloneCommand("mkdir " + myRemoteOutputFolder());
    rcloneCommand("mkdir " + remoteSignalsFromRpiFolder());
    rcloneCommand("mkdir " + remoteSignalsToRpiFolder());
    rcloneCommand("mkdir " + myremoteSignalsFromRpiFolder());
    rcloneCommand("mkdir " + myremoteSignalsToRpiFolder());
    rcloneCommand("mkdir " + myremoteFlashFileFolder());
}

void uploadData(){
    cout << "Uploading\n";
    publishMe();

    /*Copy data such that rclone doesn't try to upload stuff being edited. We only do this if we haven't spent our 10mb */
    //if (checkLogSizes()){
    /*Actual uploads*/
        string localpath = localOutputUploadFolder();
        string remotepath = myRemoteOutputFolder();
        rcloneCommand("sync " + localpath + " " + remotepath + " --create-empty-src-dirs");
        std::cout << std::string("sync " + localpath + " " + remotepath + " --create-empty-src-dirs\n");
        localpath = localSigFromRpiFolder();
        remotepath = myremoteSignalsFromRpiFolder();
        rcloneCommand("sync " + localpath + " " + remotepath + " --create-empty-src-dirs");
    //}
    /*Ensuring that a signals folder will be online, as well as sending the liveness signal*/
    rcloneCommand("touch " + myRemoteLiveSignalPath());
}



/*Creates necessary directories if needed*/
bool directoryCheck(){
    string mydir = getMyDirectory();

    string paths_needed[3 + LOGCOUNT];

    /*We need one folder for storing output, one for downloading auxiliary things, and one for upload*/
    paths_needed[0] = localOutputFolder();
    paths_needed[1] = localOutputUploadFolder();
    paths_needed[2] = localSigFromRpiFolder();
    paths_needed[3] = localSigToRpiFolder();

    for (int i = 0; i < 4; i++){
        /*We do a reset */
        string command = "rm -rf " + paths_needed[i];
        system(command.c_str());
        /*We create the folders */
        command = "mkdir -p " + paths_needed[i];
        system(command.c_str());
    }

    /*Ensure that we are very much online*/
    publishMe();

    return true;
}

/*Restes logs, moves log1 to log2, log2 to log3 etc*/
/*void resetLogs(){
    string deleteLastFolder = "rm -rf " + logFileFolder() + "/log" + to_string(LOGCOUNT - 1);
    system(deleteLastFolder.c_str());

    for (int i = LOGCOUNT - 2; i >= 0; i--){*/
        /*Rename folder log_i to log_i+1*/
/*        string cmd = "mv " + logFileFolder() + "/log" + to_string(i) + " " + logFileFolder() + "/log" + to_string(i + 1);
        system(cmd.c_str());
    }
    string deleteContentOfFirstFolder = "rm -rf " + logFileFolder() + "/log" + to_string(1) + "/*";
    system(deleteContentOfFirstFolder.c_str());
}
*/

bool checkGrabSerialAlive(){
    /*If we have less than two processes running it's no good*/
    return (getGrabSerialProcessArray().size() == 1);
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
    AppendToPiLog("Resetting");

    system(string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"init reset init reset run exit\"").c_str());
}

void stopMCU(){
    system(string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"init reset init reset run exit\"").c_str());
}

void flashAndStopMCU(){
    AppendToPiLog("Flashing");
    /*This implies trying to flash all files. SO ONLY LEAVE 1 FILE IN THE DIRECTORY */
    int flashed = 0;
    for (const auto & entry : filesystem::directory_iterator(localFlashFileFolder())){

        exec(string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"program " + string(entry.path()) + " verify reset exit\"").c_str());

        flashed = 1;
    }
    if (!flashed){
        resetMCU();/*We push the reset button*/
    }
    /*Delete all files in folder*/
    system(string("rm -rf " + localFlashFileFolder() + "/*").c_str());
    /*Delete all files online*/
    rcloneCommand("delete " + myremoteSignalsToRpiFolder() + FLASHFILEFOLDER);
    rcloneCommand("touch " + myremoteSignalsFromRpiFolder() + "/flashed.sig");

//    stopMCU();
}

bool newExperimentFlag(){
    if (fileExists(localNewExperimentFlagPath())){
            AppendToPiLog("New Experiment Flag Seen\n");
        deleteFile(localNewExperimentFlagPath());
        rcloneCommand("deletefile " + myRemoteNewExperimentFlagPath());
        return true;
    }
    return false;
}

bool goFlag(){
    if (fileExists(localGoFlagPath())){
        deleteFile(localGoFlagPath());
        deleteRemote(myRemoteGoFlagPath());
        return true;
    }
    return false; 
}

/*flash and signals*/

void downloadData(){
    cout << "Downloading\n";
    string localpath = localSigToRpiFolder();
    string remotepath = myremoteSignalsToRpiFolder();
    string retval;
    int attempts = 0;
        do {
            retval = exec(string("rclone sync " + remotepath + " " + localpath).c_str());
            attempts++;
        } while(retval.find(string("ERROR")) == std::string::npos && (attempts < 3));
}

void programLoop(){
    auto start = chrono::steady_clock::now();

    while(true){

        while (!checkForDevice()){
            /* We wait until we have a device */
            this_thread::sleep_for(chrono::seconds(30));
        }
        cout << "uploading\n";
        uploadData();

        cout << "downloading\n";
        downloadData();

        if (newExperimentFlag()){
            downloadData();

            deleteRemote(myRemoteOutputFolder());
            stopMCU();
            system("pkill grabserial");
            deleteFolder(localOutputFolder() + "/*");
            startGrabSerial();
            flashAndStopMCU();
        }

        /*We can spam the servers as much as we want on eduroam, so we upload often*/
        this_thread::sleep_for(chrono::seconds(2)); 
    }
}

int main(){

    currentLog = 0;
    bool macSucces = getMacAddress(mac);

    directoryCheck();
    resetGrabSerial();    
    checkForExistingLogs();
    uploadData(); /*We'll just show we're online*/    
    if (checkForDevice()){
         resetMCU();
    }


    /*//For testing          
    cout << myremoteSignalsFromRpiFolder() << endl;
    cout << localSigFromRpiFolder() << endl;
    cout << myRemoteOutputFolder() << endl;
    cout << localOutputFolder() << endl;
    cout << myRemoteGoFlagPath() << endl;
    cout << localGoFlagPath() << endl;
    cout << myRemoteNewExperimentFlagPath() << endl;
    cout << localNewExperimentFlagPath() << endl;
    cout << myRemoteLiveSignalPath() << endl;
    cout << localFlashFileFolder() << endl;
    */
    programLoop();

    //cout <<  getMyDirectory() << endl;

    return 0;
}

