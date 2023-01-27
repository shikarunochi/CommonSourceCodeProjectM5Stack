set PATH=%PATH%;C:\Users\%USERNAME%\.platformio\penv\Scripts

cd src

cd m5
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\m5.bin
cd ..

cd MSX1
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\msx1.bin
cd ..

cd pyuta
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\pyuta.bin
cd ..

cd sc3000
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\sc3000.bin
cd ..

cd jr100
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\jr100.bin
cd ..

cd pc6001
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\pc6001.bin
cd ..

cd pc6001mk2
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\pc6001mk2.bin
cd ..

cd pc6601
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\pc6601.bin
cd ..


cd MSX2
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\msx2.bin
cd ..

cd scv
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\scv.bin
cd ..

cd colecovision
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\colecovision.bin
cd ..

cd pc8201
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\pc8201.bin
cd ..

cd bmjr
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\bmjr.bin
cd ..

cd fm7
platformio run
copy /y .pio\build\m5stack-fire\firmware.bin ..\..\bin\fm7.bin
cd ..

cd mz1200
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\mz1200.bin
cd ..

cd mz1500
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\mz1500.bin
cd ..

cd mz700
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\mz700.bin
cd ..

cd rx78
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\rx78.bin
cd ..

cd x1
platformio run
copy /y .pio\build\m5stack-fire\firmware.bin ..\..\bin\x1.bin
cd ..

cd mz80b
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\mz80b.bin
cd ..

cd mz2000
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\mz2000.bin
cd ..

cd smc777
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\smc777.bin
cd ..

cd pasopia7
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\pasopia7.bin
cd ..

cd pasopia
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\pasopia.bin
cd ..

cd fp1100
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\fp1100.bin
cd ..

cd multi8
platformio run
copy /y .pio\build\m5stack-fireLGFX\firmware.bin ..\..\bin\multi8.bin

cd ..

cd ..

pause