FROM python:3.13.2-slim AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    wget \
    pkg-config \
    libssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Update git to use HTTPS instead of git protocol
RUN git config --global url."https://".insteadOf git://

# Clone, build, and install radare2
# The install script will place the final files in /usr/local/
RUN git clone https://github.com/radareorg/radare2 \
    && cd radare2 \
    && sys/install.sh


# --- Stage 2: The Final Image ---
# This is our clean, final application image
FROM python:3.13.2-slim

# Install only the RUNTIME dependencies
RUN apt-get update && apt-get install -y \
    graphviz \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy the compiled radare2 binaries and libraries from the builder stage
COPY --from=builder /usr/local/bin/r* /usr/local/bin/
COPY --from=builder /usr/local/lib/libr_*.so* /usr/local/lib/
COPY --from=builder /usr/local/lib/pkgconfig /usr/local/lib/pkgconfig
COPY --from=builder /usr/local/share/radare2 /usr/local/share/radare2

# Update the library cache
RUN ldconfig

# Set working directory
WORKDIR /app

# Copy requirements first to leverage Docker cache
COPY requirements.txt .

# Install Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Copy the rest of the application
COPY . .

# Create necessary directories
RUN mkdir -p logs generated_codes out workspace

# Set environment variables
ENV PYTHONUNBUFFERED=1
ENV LOG_LEVEL=INFO
ENV DEFAULT_ANALYSIS_MODE=standard

# Expose Streamlit port
EXPOSE 8501

# Command to run the Streamlit application
ENTRYPOINT ["streamlit", "run", "app.py", "--server.address", "0.0.0.0"]