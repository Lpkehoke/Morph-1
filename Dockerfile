FROM ubuntu:bionic
RUN apt-get update \
    && apt-get -y install g++-8 mesa-common-dev libegl1-mesa-dev libgbm-dev \
       libdrm-dev libasound2-dev libjack-dev libpulse-dev libaudio-dev libx11-dev \
       libxext-dev libxcursor-dev libxinerama-dev libxi-dev libxrandr-dev libxss-dev \
       libxxf86vm-dev pkg-config cmake ninja-build python3 python3-pip python3-dev

RUN pip3 install conan meson jinja2 \
    && conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan \
    && conan user

COPY . morph
CMD cd ./morph && ./docker-build.sh
