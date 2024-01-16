# Install dependency
```sh
apt-get -y install python-is-python3 lsof gcc g++ make cmake build-essential git autoconf automake default-mysql-client redis-tools curl argon2 telnet libtool libtool-bin libssl-dev libcurl4-openssl-dev zlib1g-dev systemd-coredump liblz4-tool libxtables-dev libip6tc-dev libip4tc-dev libiptc-dev libavformat-dev liblua5.1-0-dev libavfilter-dev libavcodec-dev libswresample-dev libevent-dev libpcap-dev libxmlrpc-core-c3-dev markdown libjson-glib-dev lsb-release libhiredis-dev gperf libspandsp-dev default-libmysqlclient-dev htop dnsutils gdb autoconf-archive gnupg2 wget pkg-config ca-certificates libjpeg-dev libsqlite3-dev libpcre3-dev libldns-dev snapd libspeex-dev libspeexdsp-dev libedit-dev libtiff-dev yasm libswscale-dev haveged jq fail2ban pandoc libre2-dev libmnl-dev libnftnl-dev libopus-dev libsndfile1-dev libshout3-dev libmpg123-dev libmp3lame-dev libopusfile-dev libgoogle-perftools-dev libboost-all-dev
```

# Build and install aws-sdk-cpp
```sh
cd aws-sdk-cpp

git submodule update --init --recursive

mkdir -p build && cd build

cmake .. -DBUILD_ONLY="lexv2-runtime;transcribestreaming" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS="-Wno-unused-parameter -Wno-error=nonnull -Wno-error=deprecated-declarations -Wno-error=uninitialized -Wno-error=maybe-uninitialized"

make -j 4 && sudo make install
```

# Build and run lex_app
### From project root
```sh
mkdir -p build && cd build
cmake ..
make -j 4
BOT_ID=<botID> BOT_ALIAS_ID=<bot-alias-id> AWS_ACCESS_KEY=<access key> AWS_SECRET_ACCESS_KEY=<secret key> ./lex_app
```