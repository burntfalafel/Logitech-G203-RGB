# using ltunify
```
./read-dev-usbmon /dev/usbmon7
```
# getting bytes from log files
```
grep -r "Send" -r <nameoffile> | sed 's/^.*=//'
```
