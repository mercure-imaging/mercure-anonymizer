FROM ubuntu:20.04
RUN apt-get update && DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata --no-install-recommends \
&& apt-get install -y build-essential dcmtk lsb-core qt5-default --no-install-recommends \
&& rm -rf /var/lib/apt/lists/*
ADD docker-entrypoint.sh bin/mercure-anonymizer ./
RUN chmod 777 ./mercure-anonymizer && chmod 777 ./docker-entrypoint.sh
CMD ["./docker-entrypoint.sh"]
