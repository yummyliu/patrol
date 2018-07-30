FROM alpine

ADD . /commander

RUN echo '@edge http://nl.alpinelinux.org/alpine/edge/community' >> /etc/apk/repositories \ 
    && apk add --upgrade --no-cache apk-tools@edge && \
    apk update && \
    apk add curl make gcc "libc-dev" "libpq" "postgresql-client" "postgresql" "postgresql-contrib" "postgresql-dev" && \
    mkdir /docker-entrypoint-initdb.d && \
    curl -o /usr/local/bin/gosu -sSL "https://github.com/tianon/gosu/releases/download/1.2/gosu-amd64" && \
    chmod +x /usr/local/bin/gosu && \
    apk del curl && \
    rm -rf /var/cache/apk/*

ENV LANG en_US.utf8
ENV PGDATA /var/lib/postgresql/data
VOLUME /var/lib/postgresql/data

COPY docker-entrypoint.sh /

ENTRYPOINT ["/docker-entrypoint.sh"]

EXPOSE 5433
CMD ["postgres"]
