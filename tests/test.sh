#!/bin/bash
x=1
while [ $x -le 10 ]
do
  curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm -e mc"
  curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xfont"
  curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm"
  curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xeglgears"
  curl -X POST http://localhost:14157/watch -d "cmd=python3 no_such_file.txt"
  sleep 1
  curl http://localhost:14157/reset
  sleep 1
  x=$(( $x + 1 ))
done
  curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xfont"
  curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm -e mc"
  curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xterm"
  curl -X POST http://localhost:14157/watch -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY LD_LIBRARY_PATH=.&cmd=xeglgears"
  curl -X POST http://localhost:14157/watch -d "cmd=python3 no_such_file.txt"



