n=0
while true;do
    >&1 echo "stdout "$n
    >&2 echo "stderr "$n
    n=$((n+1))
    sleep 1
done
