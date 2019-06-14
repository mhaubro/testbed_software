
#ifndef AUXILIARY_H
#define AUXILIARY_H

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
#include <fstream>

using namespace std;
string getRemoteDirectory();
string getMyDirectory();

void rcloneCommand(string cmd);
void deleteRemote(string path);
void purgeRemote(string path);
void deleteFile(string path);
void deleteFolder(string path);

string remoteSignalsFromRpiFolder();
string remoteSignalsToRpiFolder();
string remoteOutputFolder();

#define SIGNAL_NEWEXPERIMENT_FILE std::string("/newexperiment.signal")
#define SIGNAL_LIVE_FILE std::string("/live.signal")
#define SIGNAL_STOPPED_FILE std::string("/stopped.signal")
#define SIGNAL_GO_FILE std::string("/go.signal")
#define INPUT_TERMINAL_FILE std::string("/terminalinput.command")
#define FLASHFILEFOLDER std::string("/flash")

#define EXPERIMENTFOLDER std::string("/output")
#define FRONTENDSIGNALFOLDER std::string("/signalsToRpis")
#define BACKENDSIGNALFOLDER std::string("/signalsFromRpis")

#define RCLONE_REMOTE std::string(" DTUHPC:")
#define REMOTEROOT RCLONE_REMOTE + std::string("testbed")

bool folderExists(string path);
bool fileExists(string path);
std::ifstream::pos_type filesize(string filename);


#endif
