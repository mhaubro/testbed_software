
#include "auxiliary.h"
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
    string command = "rm " + path;
    system(command.c_str());
}

void rcloneCommand(string cmd){
    //system(string("rclone " + cmd).c_str());
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

std::ifstream::pos_type filesize(string filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg(); 
}