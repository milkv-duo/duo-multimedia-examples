# Milk-V Duo Multimedia Examples
[English](./README.md) | 简体中文

本工程提供了在 Linux 环境下使用 C/C++ 开发多媒体应用的一些例子，可以在 `Milk-V Duo 系列` 设备上运行。

## 开发环境

1. 使用本机 Ubuntu 系统，推荐 `Ubuntu 22.04 LTS`
  <br>
  (也可以使用虚拟机中的 Ubuntu 系统、Windows 中 WSL 安装的 Ubuntu、基于 Docker 的 Ubuntu 系统)

2. 安装编译依赖的工具
    ```
    sudo apt-get install wget git cmake
    ```

3. 下载交叉编译工具
    ```
    git clone https://github.com/milkv-duo/host-tools.git
    ```

4. 获取 Examples
    ```
    git clone https://github.com/milkv-duo/duo-media-examples.git
    ```

5. 修改cmake/musl_riscv64.cmake文件里CROSS_CHAIN_PATH
    ```
    # Fixme replace with your appropriate path.
    set(CROSS_CHAIN_PATH  "XXX/host-tools/gcc/riscv64-linux-musl-x86_64")
    ```

6. 编译测试
  以`video_recorder`为例，进入该例子目录直接执行 ./build_riscv64.sh 即可：
    ```
    cd video_recorder
    ./build_riscv64.sh
    ```
    编译成功后将out目录下生成的 `videoRecorder` 可执行程序通过网口或者 USB-NCM 网络等方式传送到 Duo 设备中。比如[默认固件](https://github.com/milkv-duo/duo-buildroot-sdk/releases)支持的 USB-NCM 方式，Duo 的 IP 为`192.168.42.1`，用户名是`root`，密码是`milkv`。
    ```
    $ scp out/videoRecorder root@192.168.42.1:/root/
    ```
    发送成功后，在 ssh 或者串口登陆的终端中运行`./videoRecorder -h`，会打印帮助信息。
    ```
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
    输入如下命令，即可从摄像头录制一段视频，时长60秒，flv格式，文件名out.flv
    ```
    ./videoRecorder -f flv -t 60 -o out.flv
    ```
  

## 如何创建自己的工程

- 根据需要，拷贝现有的例子，稍加修改即可。

## 各例子说明

1. video_recorder
    
    从摄像头录制一段视频。

2. video_player_vo

   采用 CVITEK 提供的Multimedia Framework 解码和播放视频。
   <br>
    注意：播放视频前要调试 [MIPI DSI接口屏幕](https://milkv.io/zh/docs/duo/low-level-dev/mipi-dsi)

3. video_player_fb

    采用 ffmpeg 解码 和 Linux framebuffer 播放视频。
    <br>
    注意：播放视频前要加载fb内核
    ```
    [root@milkv-duo]~# insmod /mnt/system/ko/cvi_fb.ko 
    ```

## 问答

### 3rd_party来源
- [cvi_mpi](https://github.com/sophgo/cvi_mpi)
- [poco](https://github.com/pocoproject/poco)
- [ffmpeg](https://github.com/FFmpeg/FFmpeg)
- [spdlog](https://github.com/gabime/spdlog)


## 关于 Milk-V

- [官方网站](https://milkv.io/)

## 技术论坛

- [MilkV Community](https://community.milkv.io/)