# sample-lex-bot-using-aws-cpp-sdk

This is a sample Lex program that uses the aws-cpp-sdk to connect to a Lex bot (preferably, the standard Airlines Bot) and streams audio of a caller saying, "I would like to book a flight".  The audio is contained in the file named I-would-like-to-book-a-flight.r8.  It is linear 16-bit PCM audio with 8khz sample rate.

As transcripts and intents are received from AWS lex they are logged to the console.  The goal is to illustrate and troubleshoot the difference in response time/latency when using the AWS SDK on Debian 12 vs Debian 11. Those issues were reported [here](https://github.com/aws/aws-sdk-cpp/issues/2779).

## Installation
### Install dependencies
```sh
sudo apt-get update
sudo apt-get -y install python-is-python3 lsof gcc g++ make cmake build-essential git autoconf automake default-mysql-client redis-tools curl argon2 telnet libtool libtool-bin libssl-dev libcurl4-openssl-dev zlib1g-dev systemd-coredump liblz4-tool libxtables-dev libip6tc-dev libip4tc-dev libiptc-dev libavformat-dev liblua5.1-0-dev libavfilter-dev libavcodec-dev libswresample-dev libevent-dev libpcap-dev libxmlrpc-core-c3-dev markdown libjson-glib-dev lsb-release libhiredis-dev gperf libspandsp-dev default-libmysqlclient-dev htop dnsutils gdb autoconf-archive gnupg2 wget pkg-config ca-certificates libjpeg-dev libsqlite3-dev libpcre3-dev libldns-dev snapd libspeex-dev libspeexdsp-dev libedit-dev libtiff-dev yasm libswscale-dev haveged jq fail2ban pandoc libre2-dev libmnl-dev libnftnl-dev libopus-dev libsndfile1-dev libshout3-dev libmpg123-dev libmp3lame-dev libopusfile-dev libgoogle-perftools-dev libboost-all-dev
```

### Build and install aws-sdk-cpp
```sh
cd /usr/local/src
sudo chmod -R a+w .
git clone https://github.com/aws/aws-sdk-cpp.git -b 1.11.217
cd aws-sdk-cpp
git submodule update --init --recursive
mkdir -p build && cd build
cmake .. -DBUILD_ONLY="lexv2-runtime;transcribestreaming" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS="-Wno-unused-parameter -Wno-error=nonnull -Wno-error=deprecated-declarations -Wno-error=uninitialized -Wno-error=maybe-uninitialized"

make  && sudo make install
```

### Build and run lex_app
```sh
cd /usr/local/src
git clone https://github.com/jambonz/sample-lex-bot-using-aws-cpp-sdk
cd sample-lex-bot-using-aws-cpp-sdk
mkdir -p build && cd build
cmake ..
make 
export BOT_ID=<botID> 
export BOT_ALIAS_ID=<bot-alias-id> 
export AWS_ACCESS_KEY=<access key> 
export AWS_SECRET_ACCESS_KEY=<secret key> 
./lex_app I-would-like-to-book-a-flight.r8
``

## Results

### Debian 11

```sh
$ ./lex_app I-would-like-to-book-a-flight.r8
Read 1948960 bytes from I-would-like-to-book-a-flight.r8 file
Received Lex Heartbeat after 80 ms
Received Lex Transcript: [{"transcript":"i would like to book a flight","eventId":"RESPONSE-3"}] after 6117 ms
1945600ms, Received Lex TextResponse: [{"eventId":"RESPONSE-5","messages":[{"msg":"I see you have a frequent flyer account with us. Can you confirm your frequent flyer number?","type":"PlainText"}]}] after 6396 ms
```

This is normal.  A transcript is returned after 6 seconds.

### Debian 12

```sh
$ ./lex_app I-would-like-to-book-a-flight.r8
Read 1948960 bytes from I-would-like-to-book-a-flight.r8 file
Received Lex Heartbeat after 88 ms
Received Lex Heartbeat after 20089 ms
Received Lex Heartbeat after 40089 ms
Received Lex Heartbeat after 60089 ms
Received Lex Heartbeat after 80089 ms
Received Lex Heartbeat after 100089 ms
Received Lex Heartbeat after 120090 ms
Received Lex Transcript: [{"transcript":"i would like to book a flight","eventId":"RESPONSE-15"}] after 129319 ms
1907200ms, Received Lex TextResponse: [{"eventId":"RESPONSE-17","messages":[{"msg":"I see you have a frequent flyer account with us. Can you confirm your frequent flyer number?","type":"PlainText"}]}] after 130214 ms
```

Here it takes over 2 minutes to retrieve a transcript.
