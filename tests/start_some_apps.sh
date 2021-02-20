#!/bin/sh
curl http://localhost:14157/reset
curl -X POST http://localhost:14157/setup -d "env=\
DISPLAY=$DISPLAY \
XAUTHORITY=$XAUTHORITY \
PATH=/opt/st/simulator/bin:/opt/st/simulator/share/postgresql/pgsql/bin:$PATH \
LD_LIBRARY_PATH=/opt/st/simulator/bin:/opt/st/simulator/lib:/opt/st/simulator/share/postgresql/pgsql/lib:$LD_LIBRARY_PATH"

#curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm -e mc"
#curl -X POST http://localhost:14157/watch -d "pwd=/bad&env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm"
#curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=bash generator_stdout_stderr.sh"
#curl -X POST http://localhost:14157/watch -d "pwd=`pwd`&cmd=./segfault"
#curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=glxgears"
curl -X POST http://localhost:14157/watch -d "cmd=glxgears"
#curl -X POST http://localhost:14157/watch -d "cmd=./glxgears"
#curl -X POST http://localhost:14157/watch -d "cmd=/bin/bash /opt/rokashevich/dog/tests/env.sh"

#curl -X POST http://localhost:14157/watch -d 'env=PATH=$PATH:/tmp/bin LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/tmp/lib&pwd=/opt/rokashevich/dog/tests&cmd=glxgears'

