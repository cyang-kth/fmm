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

RUN wget https://github.com/OSGeo/PROJ/releases/download/9.2.1/proj-9.2.1.tar.gz && \
    tar xvzf proj-*.tar.gz && \
    pushd proj-* && \
        mkdir -p build && pushd build && cmake .. && make -j8 && make install && popd && \
    popd && rm -rf proj-*

RUN wget https://github.com/OSGeo/gdal/releases/download/v3.7.0/gdal-3.7.0.tar.gz && \
    tar xvzf gdal-*.tar.gz && \
    pushd gdal-* && \
        mkdir -p build && pushd build && cmake .. && make -j8 && make install && popd && \
    popd && rm -rf gdal-*

RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.bz2 && \
    tar -xvjf boost_*.tar.bz2 && \
    pushd boost_* && \
        ./bootstrap.sh && ./b2 install && \
    popd && rm -rf boost_*
