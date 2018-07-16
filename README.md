# Freedom E SDK移植用フォークプロジェクト

Freedom E SDKをSiFiveがサポートしているFPGAボード以外のボードに移植、日本語説明の追加のプロジェクトです。
以下、和訳。

# README #

このリポジトリは、SiFive, Incによってメンテナンスされた、Freedom E RISC-Vプラットフォーム向けソフトウェアの開発を容易にするものです。

### 内容 ###

* RISC-V ソフトウェア・ツールチェイン
* RISC-V デバッグ・ツールチェイン
* FE310用ボード・サポート・パッケージと、開発キット
* いくつかの実例のプログラム

### SDKの環境構築 ###

第一に、このリポジトリをクローンします:

```
git clone --recursive https://github.com/horie-t/freedom-e-sdk.git
```
(訳注: ここでは、このフォークリポジトリをクローンするように記述しています。)

Makeファイルのオプションを見るには:

```
cd freedom-e-sdk
make help
```

#### ソースコードからツールをビルド ####

Ubuntuのパッケージで必要なもの:

	$ sudo apt-get install autoconf automake libmpc-dev libmpfr-dev libgmp-dev gawk bison flex texinfo libtool libusb-1.0-0-dev make g++ pkg-config libexpat1-dev zlib1g-dev  

次に、ツールをビルドします:

```
cd freedom-e-sdk
make tools [BOARD=freedom-e300-hifive1]
```

あなたのマシンに余力があるなら、`make` に `-j n` を加えてビルド処理の速度アップができます。ここで `n` は、あなたビルドシステムで使えるプロセッサの数です。

#### ビルド済みバイナリツールの利用 ####

ソースからツールをコンパイルしたくない場合は、下記からビルド済みバイナリが利用できます。

https://www.sifive.com/products/tools

OpenOCD、RISC-V GNU Toolchainについては、あなたのプラットフォームにあった、.tar.gzをダウンロードし、配置したい場所で伸長して下さい。そして、 `RISCV_PATH` と `RISCV_OPENOCD_PATH` 環境変数を設定して、ツールを使用します。

```
cp openocd-<date>-<platform>.tar.gz /my/desired/location/
cp riscv64-unknown-elf-gcc-<date>-<platform>.tar.gz /my/desired/location
cd /my/desired/location
tar -xvf openocd-<date>-<platform>.tar.gz
tar -xvf riscv64-unknown-elf-gcc-<date>-<platform>.tar.gz
export RISCV_OPENOCD_PATH=/my/desired/location/openocd
export RISCV_PATH=/my/desired/location/riscv64-unknown-elf-gcc-<date>-<version>
```

### SDKの更新 ###

SDKを最後のバージョンに更新したい場合:

```
cd freedom-e-sdk
git pull origin master
git submodule update --init --recursive
```

上記の実施後、全ツールチェインを再コンパイルしたい場合:

```
make uninstall
make tools
```

### ツールの利用方法 ###

ベアメタルでRISC-Vプログラムをコンパイルするには:

```
cd freedom-e-sdk
make software [PROGRAM=demo_gpio] [BOARD=freedom-e300-hifive1]
```

コマンドの詳細情報については、 `make help` を実行して下さい。

### Benchmarking ###

#### Dhrystone ####

After setting up the software and debug toolchains, you can build and
execute everyone's favorite benchmark as follows:

- Compile the benchmark with the command `make software BOARD=freedom-e300-hifive1 PROGRAM=dhrystone LINK_TARGET=dhrystone`. Note that a slightly different linker file is used for Dhrystone which stores read only data in DTIM instead of external flash.
- Run on the HiFive1 board with the command `make upload BOARD=freedom-e300-hifive1 PROGRAM=dhrystone`.
  This will take a few minutes.  Sample output is provided below.
- Compute DMIPS by dividing the Dhrystones per Second result by 1757, which
  was the VAX 11/780's performance.  In the example below, 729927 / 1757 =
  415 DMIPS.
- Compute DMIPS/MHz by dividing by the clock rate: in the example below,
  415 / 268 = 1.55 DMIPS/MHz.

```
core freq at 268694323 Hz

Dhrystone Benchmark, Version 2.1 (Language: C)

<snip>

Microseconds for one run through Dhrystone: 1.3
Dhrystones per Second:                      729927.0
```

#### CoreMark ####

We cannot distribute the CoreMark benchmark, but following are instructions
to download and run the benchmark on the HiFive1 board:

- Download CoreMark from EEMBC's web site and extract the archive from
  http://www.eembc.org/coremark/download.php.
- Copy the following files from the extracted archive into the
  `software/coremark` directory in this repository:
  - `core_list_join.c`
  - `core_main.c`
  - `coremark.h`
  - `core_matrix.c`
  - `core_state.c`
  - `core_util.c`
- Compile the benchmark with the command `make software PROGRAM=coremark`.
- Run on the HiFive1 board with the command `make upload PROGRAM=coremark`.
- Divide the reported Iterations/Sec by the reported core frequency in MHz to
  obtain a CoreMarks/MHz value.

### For More Information ###

Documentation, Forums, and much more available at

[dev.sifive.com](https://dev.sifive.com)
