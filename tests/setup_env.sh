export DISPLAY=$DISPLAY
export XAUTHORITY=$XAUTHORITY
export PATH=/opt/st/simulator/bin:/opt/st/simulator/share/postgresql/pgsql/bin:/bin:/usr/bin:/usr/local/bin
export LD_LIBRARY_PATH=/opt/st/simulator/bin:/opt/st/simulator/lib:/opt/st/simulator/share/postgresql/pgsql/lib
export PWD=/opt/st/simulator/bin
curl -X POST 127.0.0.1:14157/setup -d "env=DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY PATH=$PATH LD_LIBRARY_PATH=$LD_LIBRARY_PATH&pwd=/opt"