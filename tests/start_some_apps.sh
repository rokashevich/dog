#!/bin/sh
#curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm -e mc"
#curl -X POST http://localhost:14157/watch -d "pwd=/bad&env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm"
#curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=bash generator_stdout_stderr.sh"
#curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=./segfault"
curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=glxgears"
#curl -X POST http://localhost:14157/watch -d "env=A=a B=b C=c&cmd=glxgears"
#curl -X POST http://localhost:14157/watch -d "cmd=glxgears"

#curl -X POST http://localhost:14157/setup -d "env=A=a B=b LD_LIBRARY_PATH=4:5:6"
#curl -X POST http://localhost:14157/watch -d 'env=C=c D=d LD_LIBRARY_PATH=1:2:3:$LD_LIBRARY_PATH X=x&pwd=`pwd`&cmd=bash env.sh'
#curl -X POST http://localhost:14157/watch -d 'cmd=/bin/bash /opt/rokashevich/dog/tests/env.sh'

