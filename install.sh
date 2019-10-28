#Installation script for the testbed-software and all that is needed. Assumes debian-based.
sudo apt update
sudo apt upgrade
sudo apt-get install grabserial automake pkg-config libtool libusb-1.0-0-dev gcc-8 g++-8 curl
git clone https://github.com/mhaubro/ti_openocd_fixed.git openocd-ti && cd openocd-ti/openocd && sudo autoreconf -f -i && sudo ./configure && make && sudo make install && sudo mv /usr/local/bin/openocd /usr/bin/openocd && cd ../..
curl https://rclone.org/install.sh | sudo bash
sudo cp testbed-software.service testbed-update-service.service /etc/systemd/system
sudo systemctl daemon-reload && sudo systemctl enable testbed-software.service && sudo systemctl enable testbed-update-service.service && sudo reboot
#Apart from this script, remember to setup an rclone remote, using the given rclone config file
