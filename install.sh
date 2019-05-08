#Installation script for the testbed-software and all that is needed. Assumes debian-based.
sudo apt-get install grabserial automake pkg-config libtool libusb-1-0-0-dev rclone
git clone https://github.com/mhaubro/ti_openocd_fixed.git openocd-ti && cd openocd-ti/openocd &&  autoreconf -f -i && ./configure && make && sudo make install && sudo mv /usr/local/bin/openocd /usr/bin/openocd
sudo apt-get install gcc-8 g++-8

