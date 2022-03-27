#### 前言

本demo是使用开源项目libmad来将MP3数据解码成PCM（16位有符号小字节序）数据。（环境：x86_64 Ubuntu16.04 64位）



### 1、编译使用

**libmad的编译：**

- 源码下载地址1：[https://sourceforge.net/projects/mad/files/libmad/](https://sourceforge.net/projects/mad/files/libmad/)

- 源码下载地址2：[https://www.linuxfromscratch.org/blfs/view/svn/multimedia/libmad.html](https://www.linuxfromscratch.org/blfs/view/svn/multimedia/libmad.html)


```bash
tar xzf libmad-0.15.1b.tar.gz
cd libmad-0.15.1b/
sed -i '/-fforce-mem/d' configure   # 如果不执行这句命令，一些编译器可能会报"gcc: error: unrecognized command line option '-fforce-mem'"错误
./configure --prefix=$PWD/_install --enable-static --disable-shared
make
make install
```

**demo的编译与使用：**

```bash
$ make clean && make
$ 
$ ./mp32pcm
Usage:
    ./mp32pcm <in MP3 file> <out PCM file>
Examples:
    ./mp32pcm audio/test1_44100_stereo.mp3 out1_44100_16bit_stereo.pcm
    ./mp32pcm audio/test2_22050_stereo.mp3 out2_22050_16bit_stereo.pcm
    ./mp32pcm audio/test3_22050_mono.mp3   out3_22050_16bit_mono.pcm
    ./mp32pcm audio/test4_8000_mono.mp3    out4_8000_16bit_mono.pcm
```



### 2、参考文章

 - [libmad linux交叉编译移植\_SongYuLong的博客的博客-CSDN博客\_libmad 交叉编译l](https://blog.csdn.net/songyulong8888/article/details/88027792)
- [基于Libmad的流媒体解码播放Demo - 简书](https://www.jianshu.com/p/287f9081218a)
- libmad-0.15.1b/minimad.c（以放到本demo中的docs/reference_code/目录。）



### 附录（demo目录架构）

```bash
$ tree
.
├── audio
│   ├── out1_44100_16bit_stereo.pcm
│   ├── out2_22050_16bit_stereo.pcm
│   ├── out3_22050_16bit_mono.pcm
│   ├── out4_8000_16bit_mono.pcm
│   ├── test1_44100_stereo.mp3
│   ├── test2_22050_stereo.mp3
│   ├── test3_22050_mono.mp3
│   └── test4_8000_mono.mp3
├── docs
│   ├── libmad linux交叉编译移植_SongYuLong的博客的博客-CSDN博客_libmad 交叉编译.mhtml
│   ├── reference_code
│   │   └── minimad.c
│   └── 基于Libmad的流媒体解码播放Demo - 简书.mhtml
├── include
│   └── mad.h
├── lib
│   └── libmad.a
├── main.c
├── Makefile
└── README.md
```