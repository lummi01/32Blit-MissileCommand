# Missile Command
![](/assets/image.png)
<BR>
A classic arcade game for 32Blit.

![](/assets/MissileCommand0.bmp)![](/assets/MissileCommand1.bmp)![](/assets/MissileCommand2.bmp)!


For local build:
```
mkdir build
cd build
cmake -D32BLIT_DIR=/path/to/32blit-sdk/ ..
make
```

For 32Blit build:
```
mkdir build.stm32
cd build.stm32
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/32blit/repo/32blit.toolchain
make
```

