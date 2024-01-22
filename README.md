my version of netcat, not unix version port. this means some anti virus program will not block it.
# WinCat

## Usage
    wc [-laksfwechvmFTUBD] [host] [port]
    
   l&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Listen TCP for incoming connections. It is an error to
                  use this option with a host specified.

   a&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;combine with l , can accept only connection which came from.
                  use this option restrict access ip.

   D&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;combine with l , Daemon processed.

   m&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;client mode send this message and then "m"

   f/F&nbsp;&nbsp;filename&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;clinet mode same as -m but message read "f" line by line/"F" Block by Block from file max 8KB.
                  and receive once only e.g.wnc -t 10 -f c:\foo.txt 127.0.0.1 2222

   t&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;combine with -m/-f to send times of message.

   T&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;combine with -t  to with multithreaded send times of message.

   k&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Keep listening. Forces wc to stay listening
                  for another connection after its current
                  connection is completed. It is an error to use
                  this option without -l.

   e&nbsp;&nbsp;filename&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Specify a filename to execute after connecting
                  (use with caution). See -c for enhanced
                  functionality. It is an error to use this option
                  with -c, -s, or -z.
                  e.g.    host A (10.0.0.2)>  wnc -l -k -e "cmd" 8118
                          host B (10.0.0.3)>  wnc 10.0.0.2 8118

   c&nbsp;&nbsp;command&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Specify a command to pass to "cmd /c" for
                  execution after connecting. It is an error
                  to use this option with -e, -s, or -z.
                  e.g.    host A (10.0.0.2)>  wnc -l -c "dir" 8118
                          host B (10.0.0.3)>  wnc 10.0.0.2 8118

   U&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;udp Mode : wnc -U 10.0.0.2 8118

   B&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;udp Mode BroadCast : wnc -B 172.30.255.255 8118

   M&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;udp Mode MultiCast : wnc -M 239.255.255.250 8118

   w&nbsp;&nbsp;n&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;client Mode Connect Send and Receive will:
                 0:close immediatly greater than wait for timeout,
                 F:will wait forever until peer disconnect.
                 O:will receive once and then disconnect.
                 R:receive only and only receive once.

   h&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Print this help page.

   oe&nbsp;&nbsp;filename&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;client mode output to file.

   pe&nbsp;&nbsp;n&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;port scan,timeout n = 1~5 in seconds.

   se&nbsp;&nbsp;filename&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;SHA1_HASH a file.

   v&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;verbose.

   host&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Can be a numerical address or a symbolic
                  hostname. If the -s option is specified, CIDR
                  notation (IPv4 only) can be used to specify a
                  range of hosts.

   port&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Must be single integer. If the -z option
                  is specified, a range of ports can be used instead.

## Building on Windows
Microsoft's Visual C++ Build Tools 
(vcvarsall/cl/nmake) are assumed to be 
installed and added to PATH.
1) Open the command prompt and navigate 
   to the WinCat directory.
2) Type `vcvarsall x86` to load the 
   windows development environment.
3) Type `nmake`.

Usage: with OTP with key "KKK"<br/>
open TCP server on port 8118<br/>
wnc -l -k -P KKK -e "cmd" 8118<br/>
open a UDP server on port 8118<br/>
wnc -l -k -U -P KKK -e "cmd" 8118<br/>

then use: otpgenerator.exe KKK to generate OTP(6 digits) and put OTP into -P <br/>
for TCP<br/>
wnc -P 802163 127.0.0.1 8118<br/>
for UDP<br/>
wnc -U -P 802163 127.0.0.1 8118<br/>
