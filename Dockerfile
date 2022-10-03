FROM emscripten/emsdk:3.1.23

ENV EM_DIR=/emsdk/upstream/emscripten
ENV EM_CACHE=/emscripten_data/cache

RUN ln -s /usr/bin/python3 /usr/bin/python \
    && npm install -g yarn
