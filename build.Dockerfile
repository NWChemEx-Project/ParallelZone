# Copyright 2023 NWChemEx-Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM ubuntu:20.04

# Modify this line to test CI workflows ########

ARG GCC_VERSION=9
ARG CLANG_VERSION=11

# Install basic tools
RUN    apt-get update \
	&& DEBIAN_FRONTEND=noninteractive apt-get install -y \
		git \
		wget \
		pip \
		gcc-${GCC_VERSION} \
		g++-${GCC_VERSION} \
		clang-${CLANG_VERSION} \
		libc++-${CLANG_VERSION}-dev \
		libc++abi-${CLANG_VERSION}-dev \
		ninja-build \
		libxml2-dev \
		libxslt-dev \
		python3-dev \
		openmpi-bin \
		libopenmpi-dev \
                docker.io \
	&& apt-get clean \
	&& pip install gcovr \
	&& pip install cppyy \
	&& rm -rf /var/lib/apt/lists/*

ARG CMAKE_VERSION=3.17.0

# Install cmake
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
      -q -O /tmp/cmake-install.sh \
      && chmod u+x /tmp/cmake-install.sh \
      && mkdir /usr/bin/cmake \
      && /tmp/cmake-install.sh --skip-license --prefix=/usr/bin/cmake \
      && rm /tmp/cmake-install.sh

ENV PATH="/usr/bin/cmake/bin:${PATH}"

# Install catch2
ARG CATCH2_VERSION=2.13.8

RUN cd /tmp \
    && git clone -b v${CATCH2_VERSION} https://github.com/catchorg/Catch2.git \
    && cd Catch2 \
    && cmake -Bbuild -H. -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=/install \
    && cmake --build build \
    && cmake --build build --target install \
    && rm -rf /tmp/Catch2

# The version no./SHAs of dependencies are from https://github.com/NWChemEx-Project/NWXCMake/blob/master/cmake/nwx_versions.cmake

# Install MADNESS
RUN cd /tmp \
    && git clone https://github.com/m-a-d-n-e-s-s/madness.git \
    && cd madness \
    && git checkout 997e8b458c4234fb6c8c2781a5df59cb14b7e700 \
    && export BUILD_TARGET=MADworld \
    && export FIND_TARGET=MADworld \
    && cmake -DENABLE_UNITTESTS=OFF -DMADNESS_BUILD_MADWORLD_ONLY=ON -DMADNESS_ENABLE_CEREAL=ON -DENABLE_MKL=OFF -DENABLE_ACML=OFF -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=/install -Bbuild . \
    && cmake --build build \
    && cmake --build build --target install \
    && rm -rf /tmp/madness

# Install spdlog
RUN cd /tmp \
    && git clone https://github.com/gabime/spdlog.git \
    && cd spdlog \
    && git checkout ad0e89cbfb4d0c1ce4d097e134eb7be67baebb36 \
    && export BUILD_TARGET=spdlog \
    && export FIND_TARGET=spdlog::spdlog \
    && cmake -DSPDLOG_INSTALL=ON -DCMAKE_INSTALL_PREFIX=/install -DCMAKE_CXX_FLAGS="-fPIC" -Bbuild . \
    && cmake --build build \
    && cmake --build build --target install \
    && rm -rf /tmp/spdlog

# Install cereal
RUN cd /tmp \
    && git clone https://github.com/USCiLab/cereal.git \
    && cd cereal \
    && git checkout tags/v1.3.0 \
    && export BUILD_TARGET=cereal \
    && export FIND_TARGET=cereal \
    && cmake -DJUST_INSTALL_CEREAL=ON -DCMAKE_INSTALL_PREFIX=/install -Bbuild . \
    && cmake --build build \
    && cmake --build build --target install \
    && rm -rf /tmp/cereal

LABEL maintainer="NWChemEx-Project" \
      description="Basic building environment for ParallelZone based on the ubuntu 20.04 image." \
      gcc_version=${GCC_VERSION} \
      clang_version=${CLANG_VERSION} \
      cmake_version=${CMAKE_VERSION} \
      catch2_version=${CATCH2_VERSION}
