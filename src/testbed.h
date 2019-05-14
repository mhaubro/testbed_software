#ifndef TESTBED_H
#define TESTBED_H

#define SIGNAL_RESET_FILE std::string("/reset.command")
#define SIGNAL_FLASH_FILE std::string("/flash.command")
#define SIGNAL_LIVE_FILE std::string("/live.command")

#define REMOTEROOT std::string("~/testbed")
#define RCLONE_REMOTE std::string(" DTUHPC:")


#define WAITTIME 1//Seconds
#define LOGCOUNT 5

#define IDLE_AFTER_MINUTES 20
#define IDLE_WAITTIME 3//Seconds
#endif