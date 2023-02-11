DEVICE=wlp0s20f3

ifconfig $DEVICE down
iwconfig $DEVICE mode managed
ifconfig $DEVICE up
service NetworkManager restart
