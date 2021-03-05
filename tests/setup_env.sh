curl -X POST http://localhost:14157/setup -d "env=\
DISPLAY=$DISPLAY \
XAUTHORITY=$XAUTHORITY \
PATH=/opt/st/simulator/bin:/opt/st/simulator/share/postgresql/pgsql/bin:$PATH \
LD_LIBRARY_PATH=/opt/st/simulator/bin:/opt/st/simulator/lib:/opt/st/simulator/share/postgresql/pgsql/lib:$LD_LIBRARY_PATH"