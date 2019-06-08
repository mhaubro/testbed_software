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

using namespace std;

//We assume the mac should only be found once. Also, this is the pure mac, without any slashes.
char mac[18];

string localBackendFolder(){
    string path = getMyDirectory() + "/backend";
    return path;
}
string localFrontendFolder(){
    string path = getMyDirectory() + "/frontend";
    return path;
}
string localOutputFolder(){
    string path = getMyDirectory() + "/output";
    return path;
}
string localFlashFileFolder(){
    string path = localFrontendFolder() + FLASHFILEFOLDER;
    return path;
}
string localGoFlagPath(){
    string path = localFrontendFolder() + SIGNAL_GO_FILE;
    return path;
}

string localNewExperimentFlagPath(){
    string path = localFrontendFolder() + SIGNAL_NEWEXPERIMENT_FILE;
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


/*If the machine is rebooted or this program restarted, we check if there is something already, to not overwrite old data*/
void checkForExistingLogs(){
/*Not implemented yet*/
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
    system(string("grabserial -v -d \"/dev/ttyACM0\" -b 115200 -w 8 -p N -s 1 -t --systime > " + localOutputFolder() + "/logacm0.txt"  + " -S -T &").c_str());
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
    /*Actual uploads*/
    string localpath = localOutputFolder();
    string remotepath = myRemoteOutputFolder();
    rcloneCommand("copy " + localpath + " " + remotepath + " --create-empty-src-dirs");
    /*Ensuring that a signals folder will be online, as well as sending the liveness signal*/
    rcloneCommand("touch " + myRemoteLiveSignalPath());
}



/*Creates necessary directories if needed*/
bool directoryCheck(){
    string mydir = getMyDirectory();

    string paths_needed[3 + LOGCOUNT];

    /*We need one folder for storing output, one for downloading auxiliary things, and one for upload*/
    paths_needed[0] = localOutputFolder();
    paths_needed[1] = localBackendFolder();
    paths_needed[2] = localFrontendFolder();

    for (int i = 0; i < 3; i++){
        string command = "mkdir -p " + paths_needed[i];
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
    system(string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"init reset run\"").c_str());
}

void stopMCU(){
    system(string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"reset halt\"").c_str());
}

void flashAndStopMCU(){

    /*This implies trying to flash all files. SO ONLY LEAVE 1 FILE IN THE DIRECTORY */
    for (const auto & entry : filesystem::directory_iterator(localFlashFileFolder())){
        system(string("openocd -s /usr/local/share/openocd/scripts/ -f board/ti_cc13x0_launchpad.cfg -c \"program " + string(entry.path()) + " verify reset exit\"").c_str());
    }
    /*Delete all files in folder*/
    system(string("rm -rf " + localFlashFileFolder() + "/*").c_str());
    /*Delete all files online*/
    deleteRemote(myremoteSignalsToRpiFolder() + FLASHFILEFOLDER);

    stopMCU();
}

bool newExperimentFlag(){
    if (fileExists(localNewExperimentFlagPath())){
        deleteFile(localNewExperimentFlagPath());
        deleteRemote(myRemoteNewExperimentFlagPath());
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
    string localpath = localFrontendFolder();
    string remotepath = remoteSignalsToRpiFolder();
    rcloneCommand("copy " + remotepath + " " + localpath);
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
            /*We pause until we get a go-flag, at which we reset*/
            terminateGrabSerial();
            uploadData();
            flashAndStopMCU();
            /*We wait for mcu flashing*/
            this_thread::sleep_for(chrono::seconds(1));
            /*We delete everything in our local folder and remote*/
            deleteRemote(myRemoteOutputFolder());
            deleteFolder(localOutputFolder());
            /*We start recording again*/
            startGrabSerial();
            resetMCU();
            start = chrono::steady_clock::now();
        }

        /*We can spam the servers as much as we want on eduroam, so we upload often*/
        this_thread::sleep_for(chrono::seconds(WAITTIME));
    }
}

int main(){

    //this_thread::sleep_for(chrono::seconds(WAITTIME));

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
    cout << localBackendFolder() << endl;
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

