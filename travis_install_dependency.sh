#!/bin/bash

if [ $TRAVIS_OS_NAME = 'osx' ]; then
  brew install boost gdal libomp expat bzip2
else
  sudo add-apt-repository -y ppa:ubuntugis/ppa;
  sudo apt-get -q update;
  sudo apt-get -y install libboost-dev libboost-serialization-dev gdal-bin libgdal-dev make cmake;
  sudo apt-get -y install libbz2-dev libexpat1-dev swig python-dev;
fi
