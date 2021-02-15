#!/bin/sh
#curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm -e mc"
#curl -X POST http://localhost:14157/watch -d "pwd=/bad&env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm"
curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=bash generator_stdout_stderr.sh"
curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=./segfault"
#curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=glxgears"
