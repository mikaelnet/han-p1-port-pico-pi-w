# han-p1-port-pico-pi-w
Experiment repo for reading the HAN-port, aka P1-port, using a Raspberry PI Pico W


# Hardware pins
Programming (see raspberrypi-swd.cfg)
RP2040 swclk -> 25  (header pin number 22)
RP2040 gnd   -> gnd (header pin number 20)
RP2040 swdio -> 24  (header pin number 18)

# Building/Programming
Setting up the build pipeline:
[https://www.cnx-software.com/2021/01/24/getting-started-with-raspberry-pi-pico-using-micropython-and-c/#c-c-on-raspberry-pi-pico]

Commands to remember
* `export PICO_SDK_PATH=/home/pi/pico/pico-sdk`
* from the build folder: `cmake -DPICO_BOARD=pico_w -DWIFI_SSID="myssid" -DWIFI_PASSWORD="mywifipassword" .. && make -j4`
* `openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg -c "program han.elf verify reset exit"`
* Terminal: `mimicom -b 115200 -o -D /dev/ttyACM0`
