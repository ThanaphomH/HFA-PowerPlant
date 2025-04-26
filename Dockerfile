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
RUN g++ -O2 run.cpp -o power_plant -I/usr/local/include/highs -L/usr/local/lib -lhighs -lz

# Set ENTRYPOINT to run the solver
ENTRYPOINT ["/app/power_plant"]

# Set command to run the solver with arguments
CMD ["/bin/sh", "-c", "./power_plant \"$@\""]



# # Use official Python slim image for a smaller footprint
# FROM python:3.10-slim

# # Set working directory
# WORKDIR /app

# # Copy the Python script
# COPY run.py .

# # Install PuLP and CBC solver
# RUN pip install pulp && \
#     apt-get update && \
#     apt-get install -y coinor-cbc && \
#     apt-get clean && \
#     rm -rf /var/lib/apt/lists/*

# # Command to run the Python script
# CMD ["python", "run.py"]