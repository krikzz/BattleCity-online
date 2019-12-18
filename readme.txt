Did you ever dream to play multiplayer online games on your Genesis/MegaDrive? This demo is exactly what you need if so! (:
This game is a Genesis port of Battle City with network play features.
Demo was tested only with Mega-ED v1, but likely will work with mega-v2/x7
Fot usb communications used extended SSF mapper.
MEGAOS v10 or later required (For Mega-ED v1)

1) Plug USB cable to Mega EverDrive
2) Run battlecity-online.bin on your genesis/megadrive. 
3) Run netclient.exe on PC. 
4) Push "NET SYNC" in game menu. 
Console will connect to PC via USB first, then netclient.exe will connect to the server and will wait for second player.
Once two players will be connected, game will restar. After that one player will control joypad1 and another one will control joypad 2.
NET SYNC menu should disappear if game synchronized successfully.

netclient.exe and netserver.exe can work under linux and mac-os. This programm written in C#, you need mono-runtime to run it on non windows machine

PAL and NTSC users can not play together, so, server uses different ports for different TV standards

netserver.exe is a game server program. There is no some public server online, so you should setup own.
By default, server listen connection to port 98, user can change destination port if run netserver.exe with arguments. For example: netserver.exe 111 server will listen port 111 in this case. 
Arguments for netclient.exe is IP address and port number: netclient.exe 192.168.0.1 111

Note that server listen two ports at same time, one port for NTSC clients and another for PAL. Pal clients connects to destination port minus one.

this is how it looks:
youtu.be/-aEIRNW7OCs
youtu.be/m6OeJYWAJ6c
