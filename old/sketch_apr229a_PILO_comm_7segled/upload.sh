sshpass -p '123123123'  scp /cygdrive/c/Users/M43507/AppData/Local/Temp/arduino_build_151047/sketch_apr229a_PILO_comm_7segled.ino.bin pi@piZero3R:~/bossac/pilo_$(date +%G-%m-%d_%H).bin
echo './bossac/enterBoot.sh' |  sshpass -p '123123123'  ssh pi@piZero3R
echo "./bossac/bossac -i -d --port=serial0 --baud=115200 -i -e -w -v  ./bossac/pilo_$(date +%G-%m-%d_%H).bin -R" |  sshpass -p '123123123'  ssh pi@piZero3R
