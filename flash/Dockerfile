# Use the Ubuntu base image
FROM ubuntu:latest

# Install dependencies
RUN apt-get update && apt-get install -y \
    wget \
    gcc \
    git \
    make \
    libncurses-dev \
    flex \
    bison \
    gperf \
    python3 \
    python3-serial \
    python3-pip \
    python-is-python3

# Create the /esp/ directory
RUN mkdir -p /esp/

# Download the file and save it to /esp/
RUN wget -O /esp/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz

# Switch to the /esp/ directory and extract the tools
WORKDIR /esp/
RUN tar -xzf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz

# Add the tools to the PATH
ENV PATH="/esp/xtensa-lx106-elf/bin:${PATH}"

# Clone ESP8266_RTOS_SDK
RUN git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git

# Set the IDF_PATH environment variable
ENV IDF_PATH /esp/ESP8266_RTOS_SDK
RUN echo $IDF_PATH

# Install Python packages from requirements.txt
RUN python3 -m pip install --user --no-cache-dir -r /esp/ESP8266_RTOS_SDK/requirements.txt
