FROM debian:10-slim

ENV EM_VERSION=1.39.10
ENV EM_DIR=/emsdk/upstream/emscripten
ENV EM_CACHE='/emscripten_data/cache'
ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8
ENV LANGUAGE en_US:en
ENV PATH=/emsdk:$EM_DIR:$PATH

RUN apt-get update && apt-get upgrade -y \
	&& apt-get install -y --no-install-recommends \
		git build-essential python libxml2 libtinfo5 ca-certificates locales \
	&& locale-gen en_US.UTF-8 \
	&& git clone https://github.com/emscripten-core/emsdk.git \
	&& mkdir -p "${EM_CACHE}" \
	&& cd emsdk && ./emsdk install ${EM_VERSION} \
	&& ./emsdk activate ${EM_VERSION} \
	&& apt-get clean autoclean \
	&& apt-get autoremove -y \
	&& rm -rf /var/lib/{apt,dpkg,cache,log}/

WORKDIR /src/
