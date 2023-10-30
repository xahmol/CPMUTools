# CPMUTools
Toolset for using the Ultimate II+ cartridge for C128 CP/M

## Contents

[Version history and download](#version-history-and-download)

[Firmware and setting prerequisites and known issues](#firmware-and-setting-prerequisites-and-known-issues)

[Introduction](#introduction)

[What is on the disk](#what-is-on-the-disk)

[CPM UConfig instructions](#CPM Uconfig-instructions)

[CPM UTime instructions](#CPM Utime-instructions)

[CPM UMount instructions](#CPM Umount-instructions)

[Credits](#credits)

![CPM UMount main interface](https://github.com/xahmol/CPMUTools/blob/main/screenshots/umount%20-%20mainscreen.png?raw=true)

## Version history and download
([Back to contents](#contents))

[Link to latest build](https://github.com/xahmol/CPMUTools/raw/main/CPMUTools-v091-20231030-1147.zip)

Version v091-20231030-1147:

- Added CPM UTime: Utility to retrieve present date and time from an NTP server and synch this time to both the Ultimate II+ Real Time Clock as the CP/M system time
- Added CPM UConfig: Utility to set the configuration options for UTime and UMount
- UMount: Bugfixes and added Save REU option
- Added documentation

Version v091-20231021-1542:

- First public alpha of CPM UMount: Utility to mount disk images stored on the Ultimate II+ filesystem in CP/M

## Firmware and setting prerequisites and known issues
([Back to contents](#contents))

### Ultimate II/II+/II+L firmware requirements

- Firmware 3.10f or newer: fully automatic drive detection and support to detect/mount/save CP/M RAM drives
- Firmware 3.xx: automatic device detection possible. But in firmware older than 3.10f automatic device detection is based on hardware drive IDs of the emulated A and B drives of the Ultimate II/II+/II+L cartridge, instead of the software ID. This might give issues if device IDs have been changed by software. To fix this, choose manual override of the automated detected targets using UConfig.
- Firmware before 3.xx: Automatic target detection is not possible, so setting valid drive targets via UConfig is always needed for CPM UMount to work.

Link to Ultimate II/II+/II+L firmware:

- Official released versions: https://ultimate64.com/Firmware
- GitHub: https://github.com/GideonZ/ultimate_releases (might contain newer not yet officially released firmware, or firmware that were temporarily retracted)

### Ultimate II/II+/II+L setting requirements

![Ultimate Cartridge Settings](https://github.com/xahmol/CPM UTools/blob/main/screenshots/u2p-settings.png?raw=true)

For CPM UTools it is important to check these settings in the C64 and Cartridge Settings menu after pressing F2 from the UI:
- Command Interface: **IMPORTANT** This setting has to be set to 'enabled' for anything in CPM UTools to work
- UltiDOS: Allow SetDate: This setting has to be set to 'enabled' to allow CPM UTime to change the Ultimate Real Time Clock time
- RAM Expansion Unit: Should be set to 'enabled' to support saving REU memory to a file.
- REU Size: set the desired size of the REU here, maximum is 16 megabytes.

## Introduction
([Back to contents](#contents))

**CPM UTools** is a little suite of (for now) three utilities to help using the Ultimate II+ cartridge from within CP/M.

### CPM UTime

Utility to set the CP/M system time (and the UII+ RTC clock) the time received from a user defined NTP server.

### CPM UMount
This utility can be used to swap and mount virtual disks on your UII+ cartridge, helpful because the UII+ menu is not available under CP/M 80 column mode.

Images can be selected from a file browser, that filters on showing only directories and disk image files.

### CPM UConfig

Utility to set the configuration for CPM UTime and CPM UMount.
For CPM UTime, enabling or disabling time query of an NTP server, the NTP server hostname, offset from UTC time in seconds and verbosity on or off can be set.
For CPM UMount, automatic valid target detection can be manually overridden, and the default target drive can be set.

## What is on the disk
([Back to contents](#contents))

In the ZIP file package of a CPM UTools build, two disk images are present, next to the PDF version of this readme file. The .d64 disk image is a C128 CP/M formatted single sided disk image, the .d71 a double sided one.
As a real 1571 drive is way faster than a 1541 drive, also the emulated 1571 drive is faster than the 1541. So if your firmware allows it, using the .d71 is recommended for higher speed. The .d64 is provided for compatibility with older firmwares (or the non-+ version of the Ultimate cartridge).

Contents of both images:

![CPM UTools disk](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20contentsfirst.png)

- **CPM UMount**: Disk mounting tool
- **CPM UTime**: NTP time synching tool
- **CPM UConfig**: Utility to set configuration options for CPM UTime

Double click on the desired icon of the three to start the corresponding utility.

Note that on first use, both CPM UTime and CPM UConfig create a configuration file on the disk if no one is yet present. After this, the contents of the disk look like this:

![Configuration file added on disk](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20contents%20data.png)

A new file called CPM UTimeDat is added to store configuration data.

Obviously, you are free (even encouraged) to copy these files to another disk target, such as your main RAM drive.

## CPM UConfig instructions
([Back to contents](#contents))

This application sets the configuration options for CPM UTIme and saves them in the CPM UTimeDat configuration file. It creates this file on first start if no file is present already.

Interface after application start, showing the present configuration settings:

![CPM UTimeCfg main interface](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20mainscreen.png?raw=true)

The options can be reached via the main menu in the top left corner. The interface will update on changed settings. All changed will be automatically saved.

Under 'CPM UMount config data' you can also see if autodetection of valid drive targets succeeded. This line can show the following options:

- Autodetection of valid drives succeeded: Firmware supports improved automated drive detection (firmware 3.10f and newer)
- Autodetection might be incorrect: Autodetection is supported, but based on hardware ID instead of software ID, so it might be wrong. Check the detected settings and manually adjust if needed (firmware 3.xx up to 3.10e)
- Autodetection not supported in firmware: Autodetection is not supported, so manual selection of valid targets is required. Detection override is forced on 'Enabled' (firmware before 3.xx)

### CP/M menu

Options in this menu:

![CP/M menu](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20CP/M%20menu.png?raw=true)

*Switch 40/80*

(Only works in CP/M128, ignored in CP/M64)

Switches between 40 and 80 column mode. On selecting, the other mode will be selected, and the screen will be redrawn. If a single monitor is used, switch monitor to the corresponding other mode to view.

*Credits*

Selecting this option shows the application credits:

![CPM UTime credits](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20CP/M%20credits.png?raw=true)

Press OK to leave the credits dialogue box.

*Exit*

Exits to the CP/M desktop and quits the application.

### Time menu

Options in this menu:

![Time menu](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20time%20menu.png?raw=true)

*Enable NTP*

Choose if time should be synched with the selected NTP server (setting is 'On'), or if only the CP/M system time should be synched to the UII+ Real Time Clock without an NTP request.

Selecting this option gives this dialogue:

![NTP enable dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20time%20ntp%20enable.png?raw=true)

Select Yes to enable, No to disable.

*Hostname*

Enter the hostname of the desired NTP server. The default server is pool.ntp.org.

This option results in this dialogue:

![Hostname dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20%20time%20hostname.png?raw=true)

For another NTP server, just enter its hostname (URL or IP). The application will not check validity of the input, to check if it works and the hostname resolves and results in a successful connection the verbose mode is suggested.

To keep the present hostname, press either CANCEL or ENTER.

*UTC offset*

Edits the time offset to UTC (Universal standard time). The offset needs to be provided in seconds. Automated adjustment for daylight savings ('Summer' and 'Winter' time) is not provided, so you have to adjust your offset on the change from daylight saving time to not.

Offset is set in seconds (to allow for half hour difference time zones and also for finetuning if desired), so multiply the UTC offset in hours by 3600. And note that the offset is signed, so start with a minus for offsets negative to UTC.

Default is set on Central European Time, which requires an offset of 3600. Setting to Central European Summer Time would require 7200. Another example: setting to Eastern Standard Time (EST, time zone for a.o. New York City) would be UTC -5, so -5*3600 = -18000.

See https://www.timeanddate.com/time/zones/ for all offsets in hours (multiply by 3600 to get to seconds).

This option presents this dialogue:

![UTC offset dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20%20time%20utc%20offset.png?raw=true)

Type the desired offset (noting the signing, so do not forget the minus sign if offsets are negative to UTC) and press ENTER. CANCEL to keep present value.

*Verbose*

Enable or disable the verbosity flag. Enabling this option makes the CPM UTime give visual feedback on the steps it performs, disabling will make CPM UTime perform its actions without any visual feedback.

Enabling is useful for debugging hostname and UTC offset settings, or if you just are curious. Disabling is recommended for day to day use as auto executing application at boot time.

Selecting this option gives this dialogue:

![Verbose dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20%20time%20verbosity.png?raw=true)

Select Yes for verbose mode, No for silent mode.

### Mount menu

Options in this menu:

![Mount menu](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20mount%20menu.png?raw=true)

*Enable override*

With this menu option you can enable or disable overriding the automated valid target detection for CPM Umount. This menu option gives this dialogue box:

![Enable override dialogie](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20mount%20override.png?raw=true)

Select Yes to enable manual override, No to use automatic target detection.

*Set drive*

With this menu option you can enable or disable drive A to D as valid target via this submenu:

![Set drive submenu](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20mount%20set%20drive.png?raw=true)

Choose the desired drive to change from this submenu. This gives this dialogue box:

![Set drive as valid dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20mount%20set%20drive%20valid.png?raw=true)

Select Yes to set the drive as valid target, No to select drive not to be a valid target.

*Set target*

With this menu option you can set the default active target drive. Without setting this manually, the target drive is set as the first encountered valid target.

You can select the target drive via the submenu:

![Set target submenu](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UConfig%20mount%20set%20target.png?raw=true)

Selecting a drive via this submenu will change the target setting. Selecting Auto will revert to just selecting the first encountered valid drive.

## CPM UTime instructions
([Back to contents](#contents))

This application is driven by the options stored in the configuration file and does not need further user interaction.

It is an auto exec application, so it should be started automatically if placed on the boot disk/drive. Therefore, it is recommended to use verbosity only incidentally and switch it off after use. It is also possible to start the application manually.

If verbosity is set to no in the configuration file options, nothing will be shown on the screen apart from the screen blanking during execution and the system time being updated after returning to the desktop.

If verbosity mode is 'On' and NTP enable is 'On', something like this will be shown on the screen:

![CPM UTime verbosity output](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM Utime%20verbose%20output.png)

This mode is meant to either debug your configuration settings, or if you are simply curious what happens:
- Connecting to the selected hostname of the desired NTP server
- Sending the data request to that server
- Reiving the data and interpreting result
- Showing UNIX epoch received
- Converting that output to time in selected time zone using the UTC offset
- Setting the UII+ Real Time Clock with that time
- Synching CP/M system time with the UII+ RTC clock.
- Confirming success

Click the Quit icon to exit.

Note that on any NTP connection error, the CP/M clock will be synched to the unchanged UII+ RTC clock as fallback.

If NTP enable is switched to off, only the synch between UII+ RTC and the CP/M system time takes place, no NTP time request will be performed.

## CPM UMount instructions
([Back to contents](#contents))

This application enables to mount disk images on the UII+ filesystem to an UII+ emulated drive or CP/M RAM drive (if enabled and supported in firmware version) in CP/M via a file browser.

### Application limitations and considerations
The application ensures only disk images can be selected to mount that correspond with the drive type of the target drive by filtering the file list to show only the corresponding image type.

This is done as CP/M crashes if the drive type of any of the active CP/M drive is changed on the fly by for example mounting a .D64 image to a 1581 emulated drive. As CP/M is not aware of the changed disk type, it will load the wrong turbo load code on the drive, which makes CP/M crash or freeze. By filtering the file list, this should be prevented.

To properly enable filtering, image filenames should end with the proper corresponding .dxx extension:

- .d64 for 1541 drive type or 1571 drive type
- .g64 for 1541 drive type or 1571 drive type
- .d71 for 1571 drive type
- .d81 for 1581 drive type
- .dnp for RAM Native drive images

Other image types are not presently supported.

Also note that file and dirnames can only be selected if they have a maximum name length of 20 characters, including extension. Reason is memory constraints: allowing for longer filenames would cause less filenames that can be loaded in memory. If you need to select dirs or files with longer names, shorten them first to 20 chars (including extension) at most.

Maximum number of files shown is dependent on the free memory on the target system used but should normally be around 250 files. If you have more valid images or subdirectories in your present directory, any entry over that number will not be shown. If you need to reach these dirs or files, consider reorganizing your dir to place the files in subdirs with fewer entries.

### Main interface

After start, this main interface is shown:

![CPM UMount main interface](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20mainscreen.png)

### Main menu

**CP/M menu**

Options in this menu:

![CP/M menu](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20CP/M%20menu.png)

*Switch 40/80*

(Only works in CP/M128, ignored in CP/M64)

Switches between 40 and 80 column mode. On selecting, the other mode will be selected, and the screen will be redrawn. If a single monitor is used, switch monitor to the corresponding other mode to view.

*Credits*

This menu option shows this dialogue:

![Credits CPM UMount](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20credits.png)

*Exit*

Exits to the CP/M desktop and quits the application.

**Save REU menu**

In this menu the contents of the REU memory can be saved to a .reu file in the presently active directory.

Select the memory size to save via this submenu:

![Save REU choose memory size to save](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UMount%20save%20reu%20menu.png?raw=true)

Then a dialogue box asks for a filename for the image to save. Enter filename or press Cancel to cancel.

![Save REU filename dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UMount%20save%20reu%20filename.png?raw=true)

If already a file with that name exists in the active directory, a new dialogue asks to confirm or cancel.

![Save REU file exists dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UMount%20save%20reu%20file%20exists.png?raw=true)

Depending on selected memory size, saving can take a while. After success, a message like this is shown.

![Save REU succes](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UMount%20save%20reu%20file%20succes.png?raw=true)


### Selecting the target drive

The target drive selection area is the upper right area in the main interface:

![Target drive area](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20drivetarget.png)

In this area you can select the target drive to mount the selected disk image to.

The text right of the icons A, B, C and D show if this drive letter can be selected as valid target to mount images to via the Ultimate II+ cartridge. Obviously only those drive letters that correspond active drives emulated by the UII+ can selected. If the drive letter is not a valid target, the text 'No target' is shown to the right of the button. If it corresponds to a valid target, the type of the emulated drive is presented (supported for now are emulated 1541, 1571 and 1581 drives).

CPM UMount presently does not support changing this drive configuration from within the application. Use the standard Configure app to change drive configuration if desired.

Pressing the icon of a valid drive target will change the target to this drive. If the drive type of the new target drive is different than the type of the old target, a refresh of the presently shown directory will be triggered to apply filtering for this new drive type.

### File browser navigation icons

This section is the lower right area of the main interface:

![Navgiation icons section](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20navicons.png)

|Icon|Function|
|---|---|
|![Back icon](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20back%20icon.png)|Pressing this icon moves the present directory shown to the parent directory of the presently selected directory.|
|![Root icon](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20back%20root.png)|Pressing this icon moves the present directory shown to the root directory of the filesystem.|
|![Home icon](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20back%20home.png)|Pressing this icon moves the present directory shown to the directory configured as home dir in the UII+ interface options.|
|![Top icon](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20back%20pageup.png)|Pressing this icon moves to the top of the file list in the present shown directory.|
|![Bottom icon](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20back%20pagedown.png)|Pressing this icon moves to the bottom of the file list in the present shown directory.|
|![Save icon](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UMount%20save%20image.png?raw=true)|Pressing this icon saves the contents of the active RAM drive to an image.|

**Save image icon**

This button only works if the selected target drive is a RAM drive, otherwise clicking this icon will be ignored.

With clicking this icon, the contents of the target RAM drive can be saved to an image file in the presently active directory. Extension will be based on the drive type of the target drive.

First a filename will be asked, enter filename, or click Cancel to cancel.

![Save Image filename dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UMount%20save%20image%20filename.png?raw=true)

If a file with this filename already exists, a confirmation dialogue box will pop up. Click Yes to confirm and overwrite the existing file, No to cancel.

![File exists dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UMount%20save%20image%20file%20exists.png?raw=true)

Depending on size of the image, saving can take a while. On success, this message will pop up. Click OK to continue.

![Image save success dialogue](https://github.com/xahmol/CPM UTools/blob/main/screenshots/CPM Utools%20-%20CPM UMount%20save%20image%20file%20succes.png?raw=true)


### File browser: pathname

![file browser path name](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20file browser%20id%20path.png)

This section shows the path of the presently selected directory, next to the identification (Ultimate DOS version) of the UII+ file system Ultimate Command Interface DOS target.

### File browser: line scroll arrows

![file browser linescroll](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20file browser%20linescroll.png)

By clicking on the arrow icons in the file browser area you can scroll one entry up (upper arrow icon) or down (lower arrow icon).

### File browser: page up and page down

![file browser page scroll](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20file browser%20pagescroll.png)

By clicking on the upper and lower scrollbar areas in the file browser area you can scroll one page up (upper scrollbar area) or down (lower scrollbar area).

### File browser: file list

![file browser file list](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20file browser%20dirs%20and%20files.png)

This area shows the visible part of the file and directory list of the presently selected directory. 13 files or directories can be shown visibly, use the scroll or navigation icons/areas to move the visible part.

In the left column the name of the file or directory is shown, in the right column the type.

Types:

![Types](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20too%20long.png)

|Code|Description|
|---|---|
|DIR|Directory|
|D64|1541 images with .d64 extension|
|D71|1571 images with .d71 extension|
|D81|1581 images with .d81 extension|
|!TL|Name too long to be fully shown. Can not be selected without shortening name|
|!IS|DNP image with a size not matching size of target drive|

**Moving to a subdirectory**

Move to a subdirectory by clicking on the desired directory name shown.

**Selecting an image to mount**

Select an image to mount by clicking on a valid filename (so type .d64, .d71 or .d81 dependent on selected target drive).

On success, the following dialogue is presented:

![Image mount dialogue](https://raw.githubusercontent.com/xahmol/CPM UTools/main/screenshots/CPM Utools%20-%20CPM UMount%20mount%20confirm.png)

If this is shown, the selected image is already mounted.

Choose No if you want to proceed with CPM UMount (for example if you selected a wrong image or if you want to mount images on other targets as well).

Choose Yes to exit the CPM UMount application and go back to the CP/M desktop with the new image mounted to the selected target drive.

## Credits
([Back to contents](#contents))

### CPMUMount
Disk mounter for the Ultimate II+ in CP/M

Written in 2023 by Xander Mol

https://github.com/xahmol/CPMUTools

https://www.idreamtin8bits.com/

Code and resources from others used:

-   Scott Hutter - Ultimate II+ library:
    https://github.com/xlar54/ultimateii-dos-lib/
    (library for the UII+ UCI functions)

-   z88dk C cross compiler for Z80 targets, including C128 CP/M
    https://github.com/z88dk/z88dk/wiki

-   Jochen Metzinger - ctools
    https://github.com/mist64/ctools

-   ntp2ultimate by MaxPlap
    https://github.com/MaxPlap/ntp2ultimate
    Time via NTP code

-   EPOCH-to-time-date-converter by sidsingh78
    https://github.com/sidsingh78/EPOCH-to-time-date-converter/blob/master/epoch_conv.c

-   Bart van Leeuwen for providing CP/M images, testing and advice

-   Gideon Zweijtzer for creating the Ultimate II+ cartridge and the Ultimate64, and the Ultimate Command Interface enabling this software.
   
-   Tested using real hardware (C128D, C128DCR) using CP/M 3.0 and ZP/M+.

Licensed under the GNU General Public License v3.0

The code can be used freely as long as you retain
a notice describing original source and author.

THE PROGRAMS ARE DISTRIBUTED IN THE HOPE THAT THEY WILL BE USEFUL,
BUT WITHOUT ANY WARRANTY. USE THEM AT YOUR OWN RISK!

