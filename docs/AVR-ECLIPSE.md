AVR Eclipse Setup Guide
=======================
To setup eclipse for AVR development you will need to folleow these steps
1. First you will need to install **Eclipse IDE for C/C++ Developers** that can be obtained from eclipse project at http://www.eclipse.org/, this include the **C/C++ Development Tools (CDT)** plug in that is requried to develop C/C++ application. If you already have an eclipse installation you can install CDT using instructions from [here](https://eclipse.org/cdt/downloads.php).
2. Once installed you will need the AVR tools, for windoes you can install [WinAVR](https://sourceforge.net/projects/winavr/files/), and for linux you will need to install following packages
    a. gcc-avr
    b. binutils-avr
    c. gdb-avr
    d. avr-libc
    e. avrdude
3. After that you just need to install eclipse AVR tools that can be obtained from http://avr-eclipse.sourceforge.net/updatesite/, please use *Help*|*Install New Software* in eclipse to do that.
4. Once setup import the *Existing Projects into Workspace* using *File*|*Import* in eclipse.