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
#include <time.h>     
#include <fstream>  

#include "auxiliary.h"
#include "sync.h"
/*More or less all setup has been done by the rpis*/
using namespace std;

void downloadData(){
    /*Data*/
    string localpath = getMyDirectory() + CURRENT_EXPERIMENT_FOLDER;
    string remotepath = remoteOutputFolder();
    rcloneCommand("copy " + remotepath + " " + localpath + " --create-empty-src-dirs");

    /*Signals*/
    localpath = getMyDirectory() + SIGNAL_FROM_RPI_FOLDER;
    remotepath = remoteSignalsFromRpiFolder();
    rcloneCommand("copy " + remotepath + " " + localpath + " --create-empty-src-dirs");
}

void uploadData(){
    string localpath = getMyDirectory() + SIGNAL_TO_RPI_FOLDER;
    string remotepath = remoteSignalsToRpiFolder();
    rcloneCommand("copy " + localpath + " " + remotepath + " --create-empty-src-dirs");
}

string logPath(){
    return getMyDirectory() + LOGFOLDERNAME;
}


string startExperimentFile(){
    string path = getMyDirectory() + START_EXPERIMENT_FILE;
    return path;
}

void resetLogs(){
    for (int i = LOGCOUNT - 2; i >= 0; i--){
        string cmd = "mv " + logPath() + to_string(i) + " " + logPath() + to_string(i + 1);
        system(cmd.c_str());
    }
    /*We store the current experiment*/
    string cmd = "mv " + getMyDirectory() + CURRENT_EXPERIMENT_FOLDER + " " + logPath() + to_string(0);
    system(cmd.c_str());
}

void deleteFlags(){
    system(string("rm -f " + remoteSignalsToRpiFolder() + "/*" + SIGNAL_GO_FILE).c_str());
    system(string("rm -f " + remoteSignalsToRpiFolder() + "/*" + SIGNAL_NEWEXPERIMENT_FILE).c_str());
}

void startExperiment(){
    /*Indicate to everyone we want to do a new experiment*/
    for (const auto & entry : filesystem::directory_iterator(remoteSignalsToRpiFolder())){
        if (!entry.is_directory()){
            continue;
        }
        /*Uploads signals of all subdirectories*/
        string path = string(entry.path());
        /*Indicate we want to do a new experiment*/
        system(string(("touch ") + string(entry.path()) + SIGNAL_NEWEXPERIMENT_FILE).c_str());
    }
    downloadData();
    uploadData();
    /*We only want to upload once, to ensure no confusion on the rpis*/
    deleteFlags();
    /*Let's wait for everyone to get this message*/
    this_thread::sleep_for(chrono::seconds(10));
    /*Download one last time*/
    downloadData();
    /*Resetting logs*/
    resetLogs();
    /*Tell all to start again*/
    for (const auto & entry : filesystem::directory_iterator(remoteSignalsToRpiFolder())){
        if (!entry.is_directory()){
            continue;
        }
        /*Uploads signals of all subdirectories*/
        string path = string(entry.path());
        /*Indicate we want to do a new experiment*/
        system(string(("touch ") + string(entry.path()) + SIGNAL_GO_FILE).c_str());
    }       
    uploadData();
    /*We delete the flags again, to get rid of the go-flags*/
    deleteFlags();
}



void readFile(string path, void * data, int bytes){
    ifstream fl(path);  
    size_t len = fl.tellg();

    if (len < bytes){
        /*We don't handle this error yet*/
        return;
    }  

    fl.seekg(0, ios::beg);

    fl.read((char *) data, bytes);  
    fl.close();  
}

void writeFile(string path, void * data, int bytes){
    ofstream fl(path, ofstream::out);  

    fl.write((char *) data, bytes);  
    fl.close();  

}

/*Removes inactive from the commands-folder, such that it is not possible to send flash-files there*/
void removeInactive(){

    vector<string> pathsToDelete;

    for (const auto & entry : filesystem::directory_iterator(remoteSignalsFromRpiFolder())){
        string macOfEntry = entry.path().string().substr(remoteSignalsFromRpiFolder().length() , string::npos);
        /*signals of all subdirectories, sent up from different rpis*/
        string liveness = string(entry.path()) + SIGNAL_LIVE_FILE;
        string warning = string(entry.path()) + WARNING_FILE_NAME;            

        if (!fileExists(liveness)){
            /*Get current time*/
            time_t currentTime;
            time(&currentTime);

            /*We check if more than a minute has passed since we wrote our warning-file. If it has, we delete this entry*/
            if (fileExists(warning)){
                time_t oldTime;
                readFile(warning, (void *) &oldTime, sizeof(oldTime));

                if ((currentTime - oldTime) > KICKOUT_OFFLINE_TIME){
                    /*We ditch both signal folders*/
                    pathsToDelete.push_back(string(entry.path()));
                    pathsToDelete.push_back(string(remoteSignalsToRpiFolder() + macOfEntry));
                    purgeRemote(remoteSignalsToRpiFolder() + macOfEntry);
                    purgeRemote(remoteSignalsFromRpiFolder() + macOfEntry);
                }

            } else {
                writeFile(warning, (void *) &currentTime, sizeof(currentTime));
            }
        } else {
            /*We delete all warning-files as well as liveness-files, and see if they're still there the next time*/
            deleteFile(warning);
            /*Prior part is base path, entry.path()... is '/mac address. We just delete everything inside the folder*/
            string remotePathFromRoot = remoteSignalsFromRpiFolder() + macOfEntry;
            deleteRemote(remotePathFromRoot);
        }
    }       

    for (int i = 0; i < pathsToDelete.size(); i++){
        string cmd = string("rm -rf ") + pathsToDelete[i];/*Delete everything in folder*/
        system(cmd.c_str());

    }

    /*We remove the live-signals, but anticipate to see them again*/
    system(string("rm -f " + remoteSignalsFromRpiFolder() + "/*" + SIGNAL_LIVE_FILE).c_str());
}

int main(){

    for (int i = 0; i < LOGCOUNT; i++){
        string command = "mkdir -p " + logPath() + to_string(i);
        system(command.c_str());
    }

    while(1){
        /*Download*/
        cout << "Download\n";
        downloadData();

        this_thread::sleep_for(chrono::seconds(15));

        cout << "Upload\n";        
        /*Upload*/
        if (fileExists(startExperimentFile())){
            deleteFile(startExperimentFile());
            startExperiment();
        }
        
        removeInactive();
    }        
    return 0;
}
