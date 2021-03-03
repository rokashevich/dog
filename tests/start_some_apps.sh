#!/bin/sh
curl http://localhost:14157/reset
curl -X POST http://localhost:14157/setup -d "env=\
DISPLAY=$DISPLAY \
XAUTHORITY=$XAUTHORITY \
PATH=/opt/st/simulator/bin:/opt/st/simulator/share/postgresql/pgsql/bin:$PATH \
LD_LIBRARY_PATH=/opt/st/simulator/bin:/opt/st/simulator/lib:/opt/st/simulator/share/postgresql/pgsql/lib:$LD_LIBRARY_PATH"

#curl -X POST http://localhost:14157/watch -d 'env=A=1 B=2 C=3&cmd=xterm -hold -e "env"'
curl -X POST http://localhost:14157/watch -d "cmd=xterm -e mc"
#curl -X POST http://localhost:14157/watch -d "pwd=/bad&cmd=xterm"
#curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=bash generator_stdout_stderr.sh"
#curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=./segfault"
curl -X POST http://localhost:14157/watch -d "cmd=glxgears"
#curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=/bin/bash ./env.sh"
