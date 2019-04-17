cd src

cd m5
rem platformio run
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\m5.bin
cd ..

cd MSX1
rem platformio run
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\msx1.bin
cd ..

cd pyuta
rem platformio run
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\pyuta.bin
cd ..

cd sc3000
rem platformio run
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\sc3000.bin
cd ..

cd jr100
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\jr100.bin
cd ..

cd pc6001mk2
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\pc6001mk2.bin
cd ..

cd MSX2
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\msx2.bin
cd ..

cd scv
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\scv.bin
cd ..

cd colecovision
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\colecovision.bin
cd ..

cd pc8201
copy .pioenvs\m5stack-fire\firmware.bin ..\..\bin\pc8201.bin
cd ..

cd ..
