# Deprecated because of the [Onboard Memory Manager](https://www.reddit.com/r/LogitechG/comments/k4nvm4/new_logitech_onboard_memory_manager/) release - which is way better to reverse engineer


This is an attempt to replace the use of [Logitech G Hub](https://www.logitechg.com/en-eu/innovation/g-hub.html), an annoying proprietary software which doesn't make any memory modifications to microcontrollers present in Logitech devices, rather polls a signal every *x* amount of time. Also it only supports Windows and MacOS making it even more frustrating to use.

This has been designed for G-203 and can be compiled using;

```
make debug
```
and run using `./G203`. This repo is incomplete and is not the end product I had hoped to deliver hence can be used for reference.
