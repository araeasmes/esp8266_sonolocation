DEVICE=wlp0s20f3

ifconfig $DEVICE down
iwconfig $DEVICE mode monitor
rfkill unblock wlan
ifconfig $DEVICE up

