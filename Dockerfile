# ── Stage 1: Build ──────────────────────────────────────────────────────────
# Official Drogon image has all dependencies pre-installed (jsoncpp, openssl, zlib, etc.)
FROM drogonframework/drogon AS builder

WORKDIR /app
COPY CMakeLists.txt .
COPY main.cc .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --parallel $(nproc)

# ── Stage 2: Runtime ─────────────────────────────────────────────────────────
# Slim image — only copy the compiled binary + runtime libs
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    libjsoncpp25 \
    zlib1g \
    libssl3 \
    libbrotli1 \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/drogon_backend ./drogon_backend

# Render injects PORT at runtime; expose default for documentation
EXPOSE 8080

CMD ["./drogon_backend"]
