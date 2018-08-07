FROM postgresql-dev

ADD . /commander

RUN apk add libstdc++ curl-dev jsoncpp-dev && \
	chmod +x /usr/local/bin/gosu && \
    rm -rf /docker-entrypoint-initdb.d && \
    mkdir /docker-entrypoint-initdb.d && \
    apk del curl && \
    rm -rf /var/cache/apk/*

ENV LANG en_US.utf8
ENV PGDATA /var/lib/postgresql/data
VOLUME /var/lib/postgresql/data

COPY docker-entrypoint.sh /

ENTRYPOINT ["/docker-entrypoint.sh"]

EXPOSE 5433
CMD ["postgres"]
