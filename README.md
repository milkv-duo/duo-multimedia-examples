# Milk-V Duo Multimedia Examples
English | [简体中文](./README-zh.md)

This project provides examples of multimedia applications developed in C/C++ for the Linux environment, which can be run on the "Milk-V Duo Series" devices.

## Development Environment

1. Use a local Ubuntu system, with Ubuntu 22.04 LTS recommended.
   (You can also use Ubuntu within a virtual machine, Windows Subsystem for Linux (WSL), or an Ubuntu system based on Docker.)

2. Install compilation dependencies:
   ```bash
   sudo apt-get install wget git cmake
   ```
3. Download the cross-compilation tools:
   ```bash
   git clone https://github.com/milkv-duo/host-tools.git
   ```

4. Obtain the Examples:
   ```bash
   git clone https://github.com/milkv-duo/duo-media-examples.git
   ```

5. Modify the `cmake/musl_riscv64.cmake` file to set `CROSS_CHAIN_PATH`:
    ```
    # Fixme replace with your appropriate path.
    set(CROSS_CHAIN_PATH  "XXX/host-tools/gcc/riscv64-linux-musl-x86_64")
    ```

6. Compile and test:
   As an example, for the `video_recorder`, navigate to the example directory and run `./build_riscv64.sh`:  
   ```bash
   cd video_recorder
   ./build_riscv64.sh
   ``` 
   After the compilation is successful, send the `out/videoRecorder` executable program to the Duo device through the network or USB-NCM. For example, in the case of USB-NCM which is supported by the default firmware, if the IP of Duo is 192.168.42.1, the username is `root` and the password is `milkv`.
   ```bash
   $ scp out/videoRecorder root@192.168.42.1:/root/
   ``` 
   Once transferred, run `./videoRecorder -h` in your ssh or serial terminal to display help information:
   ```bash
    [root@milkv]~# ./videoRecorder -h
    usage: videoRecorder [-h] [-f flv] [-t 60] [-o out.flv]
    This is a simple video recording program based on milkV duo.

    --daemon                          Run application as a daemon.
    --umask=mask                      Set the daemon's umask (octal, e.g. 027).
    --pidfile=path                    Write the process ID of the application to 
                                      given file.
    -h, --help                        Display help information.
    -f<format>, --format=<format>     Specify output format: h264, flv.
    -t<seconds>, --time=<seconds>     Specify recording duration seconds.
    -o<outpath>, --outpath=<outpath>  Specify output path.

    For more information, visit the website https://milkv.io.
   ```
   To record a video from the camera for 60 seconds in flv format with the output filename as `out.flv`, use the following command:
   ```bash
   ./videoRecorder -f flv -t 60 -o out.flv
   ``` 
   
## How to create your own project
   - Copy an existing example and modify it according to your needs. 
   
## Example descriptions 

1. video_recorder 
   - Record a video from the camera. 

2. video_player_vo 
   - Uses CVITEK's Multimedia Framework for video decoding and playback.
   - Note: Before playing videos, configure [MIPI_DSI](https://milkv.io/zh/docs/duo/low-level-dev/mipi-dsi)
3. video_player_fb 
   - Uses ffmpeg for video decoding and Linux framebuffer for playback.
   - Note: Before playing videos, load the fb kernel module:
```bash
   [root@milkv-duo]~# insmod /mnt/system/ko/cvi_fb.ko 
   ``` 

## Questions

### Source of 3rd_party
   - [cvi_mpi](https://github.com/sophgo/cvi_mpi)
   - [poco](https://github.com/pocoproject/poco)
   - [ffmpeg](https://github.com/FFmpeg/FFmpeg)
   - [spdlog](https://github.com/gabime/spdlog)
   
## About Milk-V 
   - Visit the [official website](https://milkv.io/) for more information. 
   
## Technical Forum 
   - Join the [MilkV Community](https://community.milkv.io/) for discussions and support.
