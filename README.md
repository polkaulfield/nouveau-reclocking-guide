# Nouveau Reclocking Guide

### Credits.
Tomasz Pawlak at the Debian forums (http://forums.debian.net/viewtopic.php?f=16&t=146141&sid=c9248fe2c9ea322c065ff9023f47f019)

### Intro. 
Nouveau supports pstates for some cards, you can reclock them and get better performance, 80% ~ of the Nvidia blobs in my case.
As it says here (https://nouveau.freedesktop.org/), reclocking is supported on GM10x Maxwell, Kepler and Tesla G94-GT218 GPUs.
I have tested it on Fedora 33 with a MSI GT 710 2GB, no problems so far.

### Testing pstates for your card.

For listing the available pstates of the 0 card, run: 
```sh
sudo cat /sys/kernel/debug/dri/0/pstate
```

You will get something like this:
```
07: core 405 MHz memory 810 MHz
0f: core 653-954 MHz memory 1600 MHz
AC: core 953 MHz memory 1600 MHz
```

In my case we have the powersaving option (0x07) and the performance one (0x0f). The last one, AC, is a buggy option, ignore it.
What we are interested in our case is the performance pstate.

To test if it works before enabling it permanently, do
```sh
echo 0f | sudo tee -a /sys/kernel/debug/dri/0/pstate
```

If you find that setting stable, lets move to the next step.

### Applying pstates at boot.
Tomasz Pawlak at the [Debian forums](http://forums.debian.net/viewtopic.php?f=16&t=146141&sid=c9248fe2c9ea322c065ff9023f47f019) made a C application that can switch between pstates and found how to apply them at boot using a systemd service.
This is important because a script wont work because we need to set the setuid bit to a binary in order to do that.

Clone this repo, compile nv_pstate.c and install it with:
```sh
gcc -O1 -s -o nv_pstate nv_pstate.c
sudo mv nv_pstate /usr/local/bin
sudo chmod u+s /usr/local/bin/nv_pstate
```

Now we need to create a file in **/etc/systemd/system/nv_pstate.service** with these contents:
```ini
[Unit]
Description=Switch nv card to performance mode (pstate)

[Service]
Type=oneshot
Restart=no
ExecStart=/usr/local/bin/nv_pstate -c0 -s0xf

[Install]
WantedBy=graphical.target
```
Where **-c0** refers to your card number and **-s0xf** refers to the pstate (we used 0f in our testing).

Now just enable the service with:
```sh
sudo systemctl daemon-reload
sudo systemctl enable nv_pstate.service
```

Reboot and you should be good to go!
