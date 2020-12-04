#!/bin/bash

if [ $TRAVIS_OS_NAME = 'osx' ]; then
  brew update;
  brew install boost gdal libomp swig || true;
else
  sudo add-apt-repository -y ppa:ubuntugis/ppa;
  sudo apt-get -q update;
  sudo apt-get -y install libboost-dev libboost-serialization-dev gdal-bin libgdal-dev make cmake;
  sudo apt-get -y install swig python-dev;
fi
