#!/bin/sh
#curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm -e mc"
#curl -X POST http://localhost:14157/watch -d "pwd=/bad&env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm"
#curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=bash generator_stdout_stderr.sh"
#curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=./segfault"
#curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=glxgears"
#curl -X POST http://localhost:14157/watch -d "env=A=a B=b C=c&cmd=glxgears"
#curl -X POST http://localhost:14157/watch -d "cmd=glxgears"

curl -X POST http://localhost:14157/setup -d "env=C=c CC=cc LD_LIBRARY_PATH=1:2:3"
curl -X POST http://localhost:14157/watch -d 'env=P=p P=pp LD_LIBRARY_PATH=$LD_LIBRARY_PATH:4:5:6:$a:$bb:$cc X=x&pwd=/opt/rokashevich/dog/tests&cmd=/bin/bash env.sh'

