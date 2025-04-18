# Use Ubuntu 22.04 as the base image
FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    git \
    libz-dev \
    && rm -rf /var/lib/apt/lists/*

# Clone and build HiGHS (specific version)
RUN git clone --branch v1.7.2 https://github.com/ERGO-Code/HiGHS.git /HiGHS \
    && cd /HiGHS \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make -j4 \
    && make install \
    && ldconfig

# Set working directory
WORKDIR /app

# Copy the C++ source code
COPY run.cpp .

# Compile the program
RUN g++ -O3 run.cpp -o power_plant -I/usr/local/include/highs -L/usr/local/lib -lhighs -lz

# Set entrypoint to run the solver
CMD ["./power_plant"]