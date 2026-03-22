FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest

# копируем только бинарник
COPY build/pillow-api /usr/local/bin/pillow-api

# бинарник исполняемый
RUN chmod +x /usr/local/bin/pillow-api

EXPOSE 8080

# конфиг будет монтироваться извне
ENTRYPOINT ["/usr/local/bin/pillow-api"]
CMD ["--config", "/etc/pillow/config.yaml"]