FROM ubuntu:16.04
RUN apt-get update && apt-get install -y \
    build-essential software-properties-common\
    libboost-dev libboost-serialization-dev libssl-dev \
    cmake \
    wget \
    make libbz2-dev libexpat1-dev swig python-dev
RUN apt purge --auto-remove -y cmake
RUN version=3.18 &&\
    build=1 &&\
    mkdir ~/temp &&\
    cd ~/temp &&\
    wget https://cmake.org/files/v$version/cmake-$version.$build.tar.gz &&\
    tar -xzvf cmake-$version.$build.tar.gz &&\
    cd cmake-$version.$build/ &&\
    ./bootstrap &&\
    make && make install
RUN add-apt-repository -y ppa:ubuntugis/ppa && apt-get -q update
RUN apt-get -y install gdal-bin libgdal-dev
RUN mkdir -p /fmm
COPY . /fmm
WORKDIR /fmm
RUN rm -rf build
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make install
EXPOSE 8080
# ENV PATH /fmm/build:$PATH
# ENV PYTHONPATH /fmm/build/python:$PYTHONPATH
CMD
