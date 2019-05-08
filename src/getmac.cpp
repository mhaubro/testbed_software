#include <fstream>
#include <string>
#include <iostream>
#include <cstring>
#include "execcmd.h"

bool ismac(std::string str, std::size_t found){
	//We didn't get a match
	if (found == std::string::npos){
		return false;
	}

	//A mac is more than 17 characters from the first ':'.
	if (str.length() < (found + 23)){
		return false;
	}

	for (int i = 0; i < 5; i++){
		if (str[found+8+3*i] != ':' ){
				std::cout << "No mac found\n";
			return false;
		}
	}
	return true;
}

//Takes an 18-byte or larger char array, prints the mac in the first 17 separated with _, instead of :, to be used for folders.
bool getMacAddress(char * uc_Mac)
{

	std::string content = exec("ifconfig");

	std::size_t found = content.find("ether ");//This should be part of the first Mac address to be found

	if (!ismac(content, found)){
		return false;
	}

	memcpy((void *) uc_Mac, (void *) &content[found+6], 17);
	for (int i = 1; i < 6; i++){
		uc_Mac[3*i-1] = '_';
	}
	uc_Mac[17] = '\0';
	return true;
}

