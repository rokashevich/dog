export DISPLAY=$DISPLAY
export XAUTHORITY=$XAUTHORITY
curl -X POST 127.0.0.1:14157/setup -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY PATH=$PATH"
curl -X POST 127.0.0.1:14157/watch -d "cmd=/home/st/dog/build/pids"
