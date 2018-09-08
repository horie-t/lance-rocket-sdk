# Lance Rocketのソフトウェア開発キット

本リポジトリは、FreedomをベースとしたRISC-Vである、[Lance Rocket](https://github.com/horie-t/lance-rocket)のソフトウェア開発キット(Software Development Kit: SDK)のリポジトリです。

ソフトウェアの一例として、[KOZOS](http://kozos.jp/kozos/) を [software/kozos/](https://github.com/horie-t/lance-rocket-sdk/tree/master/software/kozos)で移植中です。(11th ステップまで完了)

サブモジュールとして、Lance Rocketのリポジトリを含みます。

## 開発環境の構築

### ソースコードのclone
まず最初に、このリポジトリを `clone` します。

```
git clone --recursive https://github.com/horie-t/lance-rocket-sdk.git
```

### ソースコードからツールをビルド

Ubuntuでは、以下をインストールが必要になります。

```
sudo apt-get install autoconf automake libmpc-dev libmpfr-dev libgmp-dev gawk bison flex texinfo libtool libusb-1.0-0-dev make g++ pkg-config libexpat1-dev zlib1g-dev
```

ツールをビルドします。

```
cd lance-rocket-sdk
make tools [BOARD=freedom-e300-nexys4ddr]
```

`make` のオプションに、 `-j n` を加えてビルド速度を上げる事ができます。ここで `n` は、ビルドするマシンのプロセッサのコアの数を指定します。

## SDKの利用方法

ベアメタルで動作するRISC-Vプログラムをコンパイルするには、以下のようにします。

```
cd lance-rocket-sdk
make software [PROGRAM=demo_gpio] [BOARD=freedom-e300-nexys4ddr]
```

