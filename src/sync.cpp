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
#include <regex>

#include "auxiliary.h"
#include "sync.h"
/*More or less all setup has been done by the rpis*/
using namespace std;

string experimentFolder(){
    return getMyDirectory() + CURRENT_EXPERIMENT_FOLDER;
}

string signalsFromRpiFolder(){
    return getMyDirectory() + SIGNAL_FROM_RPI_FOLDER;
}

string signalsToRpiFolder(){
    return getMyDirectory() + SIGNAL_TO_RPI_FOLDER;
}

string flashFilesToRpiFolder(){
    return getMyDirectory() + FLASHFOLDERNAME;
}

void downloadData(){
    /*Data*/
    string localpath = experimentFolder();
    string remotepath = remoteOutputFolder();
    rcloneCommand("sync -vv " + remotepath + " " + localpath + " --create-empty-src-dirs");

    /*Signals*/
    localpath = signalsFromRpiFolder();
    remotepath = remoteSignalsFromRpiFolder();
    rcloneCommand("sync -vv " + remotepath + " " + localpath + " --create-empty-src-dirs");
}

void deleteFlashFiles(){
    /*Deletes all files in all mac-folders */
    system(string("rm -f " + flashFilesToRpiFolder() + "/*" + "/*").c_str());
}

void uploadFlashes(){
    for (const auto & entry : filesystem::directory_iterator(flashFilesToRpiFolder())){
        string macOfEntry = entry.path().string().substr((flashFilesToRpiFolder()).length() , string::npos);
        string localPath = flashFilesToRpiFolder() + macOfEntry;
        string remoteFlashPath = remoteSignalsToRpiFolder() + macOfEntry + FLASHFILEFOLDER;
        rcloneCommand("sync --copy-links -vv " + localPath + " " + remoteFlashPath);
    }
}

void uploadData(){
    uploadFlashes();

    string localpath = signalsToRpiFolder();
    string remotepath = remoteSignalsToRpiFolder();
    rcloneCommand("copy -vv " + localpath + " " + remotepath + " --create-empty-src-dirs");
}

string logPath(){
    return getMyDirectory() + LOGFOLDERNAME;
}


string startExperimentFile(){
    string path = getMyDirectory() + START_EXPERIMENT_FILE;
    return path;
}

void resetLogs(){
    string cmd = "rm -rf " + logPath() + to_string(LOGCOUNT-1);
    system(cmd.c_str());
    

    for (int i = LOGCOUNT - 2; i >= 0; i--){
        cmd = "mv " + logPath() + to_string(i) + " " + logPath() + to_string(i + 1);
        system(cmd.c_str());
    }

    vector<string> pathsToDelete;
    /*If an output-folder is not 'alive', in our signalsFromRpis, we destroy it*/
    for (const auto & entry : filesystem::directory_iterator(experimentFolder())){
        string signalsPath = regex_replace(string(entry.path()), regex(experimentFolder()), signalsFromRpiFolder());
        if (!folderExists(signalsPath)){
            cout << "Folder missing\n";
            cout << "deleting " << regex_replace(string(entry.path()), regex(experimentFolder()), remoteOutputFolder()) << endl;
            /*Regex replaces path until mac address local with path until mac on the remote*/
            purgeRemote(regex_replace(string(entry.path()), regex(experimentFolder()), remoteOutputFolder()));
        }
    }

    /*We store the current experiment*/
    cmd = "mv " + getMyDirectory() + CURRENT_EXPERIMENT_FOLDER + " " + logPath() + to_string(0);
    system(cmd.c_str());
}

void deleteFlags(){
    system(string("rm -f " + signalsToRpiFolder() + "/*" + SIGNAL_GO_FILE).c_str());
    system(string("rm -f " + signalsToRpiFolder() + "/*" + SIGNAL_NEWEXPERIMENT_FILE).c_str());

}

void startExperiment(){
    /*Indicate to everyone we want to do a new experiment*/
    uploadData();
    //deleteFlashFiles();
    for (const auto & entry : filesystem::directory_iterator(signalsToRpiFolder())){
        // if (!entry.is_directory()){
        //     continue;
        // }
        /*Uploads signals of all subdirectories*/
        string path = string(entry.path());
        /*Indicate we want to do a new experiment*/
        string macOfEntry = entry.path().string().substr((signalsToRpiFolder()).length() , string::npos);
        rcloneCommand(string(("touch ") + remoteSignalsToRpiFolder() + macOfEntry + SIGNAL_NEWEXPERIMENT_FILE).c_str());
    }
    uploadData();
    /*We only want to upload once, to ensure no confusion on the rpis*/
    deleteFlags();
    /*Resetting logs*/
    resetLogs();
    /*Tell all to start again*/
    // for (const auto & entry : filesystem::directory_iterator(getMyDirectory() + SIGNAL_TO_RPI_FOLDER)){
    //     if (!entry.is_directory()){
    //         continue;
    //     }
    //     /*Uploads signals of all subdirectories*/
    //     string path = string(entry.path());
    //     /*Indicate we want to do a new experiment*/
    //     system(string(("touch ") + string(entry.path()) + SIGNAL_GO_FILE).c_str());
    // }       
    // uploadData();
    /*We delete the flags again, to get rid of the go-flags*/
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

    for (const auto & entry : filesystem::directory_iterator(signalsFromRpiFolder())){
        string macOfEntry = entry.path().string().substr((signalsFromRpiFolder()).length() , string::npos);
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
                    pathsToDelete.push_back(string(signalsFromRpiFolder() + macOfEntry));
                    pathsToDelete.push_back(string(signalsToRpiFolder() + macOfEntry));
                    pathsToDelete.push_back(string(flashFilesToRpiFolder() + macOfEntry));
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
            string remotePathFromRoot = remoteSignalsFromRpiFolder() + macOfEntry + SIGNAL_LIVE_FILE;
            rcloneCommand("deletefile -vv " + remotePathFromRoot);

            /*We ensure we have a upload-folder. Full path is found by substituting download-root with upload-root. We need somewhere to dump flashes*/
            string uploadFolder = regex_replace(string(entry.path()), regex(signalsFromRpiFolder()), signalsToRpiFolder());
            string flashFolder = flashFilesToRpiFolder() + macOfEntry;
            system(string("mkdir -p " + uploadFolder).c_str());
            system(string("mkdir -p " + flashFolder).c_str());
            deleteFile(liveness);
        }
    }       

    /*We remove the live-signals, but anticipate to see them again*/
    system(string("rm -f " + signalsFromRpiFolder() + "/*" + SIGNAL_LIVE_FILE).c_str());
}

void initialDownload(){
    /*Signals*/
    string localpath = signalsToRpiFolder();
    string remotepath = remoteSignalsToRpiFolder();
    rcloneCommand("copy " + remotepath + " " + localpath + " --create-empty-src-dirs");
}

int main(){

    for (int i = 0; i < LOGCOUNT; i++){
        string command = "mkdir -p " + logPath() + to_string(i);
        system(command.c_str());
    }
    
//    initialDownload();

    while(1){
        /*Download*/
        cout << "Download\n";
        downloadData();
        removeInactive();

        this_thread::sleep_for(chrono::seconds(15));

        cout << "Upload\n";        
        /*Upload*/
        if (fileExists(startExperimentFile())){
            deleteFile(startExperimentFile());
            startExperiment();
            this_thread::sleep_for(chrono::seconds(10));
        }
        
    }        
    return 0;
}
