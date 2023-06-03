FROM ghcr.io/cubao/build-env-manylinux2014-x64:v0.0.2
RUN yum install -y \
    boost-devel \
    bzip2-devel \
    expat-devel \
    gdal-devel \
    openssl-devel

RUN wget https://download.osgeo.org/libtiff/tiff-4.5.0.tar.xz && \
    tar --xz -xvf tiff-*.tar.xz && \
    pushd tiff-* && \
        mkdir -p build && pushd build && cmake .. && make -j8 && make install && popd && \
    popd && rm -rf tiff-*

# RUN add-apt-repository -y ppa:ubuntugis/ppa && apt-get -q update
# RUN apt-get -y install gdal-bin libgdal-dev
