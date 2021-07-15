[#](#) using ltunify
```
./read-dev-usbmon /dev/usbmon7
```
# getting bytes from log files
```
grep -r "Send" -r <nameoffile> | sed 's/^.*=//'
```
# getting number of repetitions
```
grep -r "Send" -r pink | sed 's/^.*=//' | grep -e "0F" | wc -l
```
