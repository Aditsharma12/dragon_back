# Single-stage build: use the official Drogon image for both build AND runtime.
# This avoids shared library mismatches between builder and runner.
FROM drogonframework/drogon

WORKDIR /app
COPY CMakeLists.txt .
COPY main.cc .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --parallel $(nproc)

EXPOSE 8080
CMD ["./build/drogon_backend"]
