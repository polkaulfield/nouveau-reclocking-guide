# Nouveau Reclocking Guide

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
Add this kernel parameter in your GRUB config. The number must be the decimal of the number you got earlier (0xf=15)
```
nouveau.config=NvClkMode=15
```

Remake grub.cfg and you should be good to go!
