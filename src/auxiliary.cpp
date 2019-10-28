
#include "auxiliary.h"
#include <sys/stat.h>
using namespace std;

string getRemoteDirectory(){
    return RCLONE_REMOTE + REMOTEROOT;
}

string getMyDirectory(){
    string homedir = getenv("HOME");
    homedir += "/testbed";
    return homedir;
}

void deleteFile(string path){
    string command = "rm -f " + path;
    system(command.c_str());
}

void rcloneCommand(string cmd){
    system(string("rclone --retries 1 " + cmd).c_str());
}

void deleteRemote(string path){
    rcloneCommand(string("delete ") + path);
}

void deleteFolder(string path){
    string command = "rm -rf " + path;
    system(command.c_str());
}

void purgeRemote(string path){
    rcloneCommand(string("purge ") + path);    
}

string remoteSignalsFromRpiFolder(){
    string path = REMOTEROOT + BACKENDSIGNALFOLDER;
    return path;
}
string remoteSignalsToRpiFolder(){
    string path = REMOTEROOT + FRONTENDSIGNALFOLDER;
    return path;
}
string remoteOutputFolder(){
    string path = REMOTEROOT + EXPERIMENTFOLDER;
    return path;
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

int filesize(string filename)
{
//    struct stat st;
//    if(stat(filename.c_str(), &st) != 0) {
        return 0;
//    }
//    return (int) st.st_size;   
}
