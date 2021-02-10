#!/bin/sh
curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm -e mc"
curl -X POST http://localhost:14157/watch -d "pwd=/root&env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm"
curl -X POST http://localhost:14157/watch -d "pwd=/bad&env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm"
curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm -e bash -c 'n=0;while true;do echo \$n;n=\$((n+1));sleep 1;done'"
curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xfontsel"
curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=glxgears"