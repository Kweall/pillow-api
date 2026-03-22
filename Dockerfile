FROM alpine:3.19 AS builder

RUN apk add --no-cache \
    cmake \
    make \
    g++ \
    git \
    curl \
    postgresql-dev \
    openssl-dev \
    zlib-dev \
    libpq \
    fmt-dev

# Клонируем userver
WORKDIR /tmp
RUN git clone https://github.com/userver-framework/userver.git
WORKDIR /tmp/userver
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) install

# Собираем наш проект
WORKDIR /app
COPY . .
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Финальный образ
FROM alpine:3.19

RUN apk add --no-cache \
    libstdc++ \
    libpq \
    openssl \
    zlib

COPY --from=builder /usr/local/lib/libuserver* /usr/local/lib/
COPY --from=builder /app/build/pillow-api /usr/local/bin/
COPY config/config.yaml /etc/pillow/config.yaml

EXPOSE 8080

CMD ["/usr/local/bin/pillow-api", "/etc/pillow/config.yaml"]