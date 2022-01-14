FROM ubuntu:20.04
RUN apt-get update && apt-get -y upgrade
RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata
RUN apt-get install -y build-essential dcmtk lsb-core qt5-default

ADD docker-entrypoint.sh ./
ADD bin/mercure-anonymizer ./
RUN chmod 777 ./mercure-anonymizer
RUN chmod 777 ./docker-entrypoint.sh

CMD ["./docker-entrypoint.sh"]
