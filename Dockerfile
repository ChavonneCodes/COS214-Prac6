FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    gdb \
    valgrind \
    cmake \
    git \
    clang \
    vim \
    nano \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

RUN cd /usr/src/googletest && \
    cmake . && \
    cmake --build . --target install && \
    rm -rf /usr/src/googletest

WORKDIR /usr/src/app

COPY . .

RUN make main scenario1 run_tests

CMD ["/bin/bash"]
