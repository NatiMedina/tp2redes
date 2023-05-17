FROM ubuntu:latest

# Instalar dependencias y herramientas necesarias
RUN apt-get update && \
    apt-get install -y build-essential gcc g++ python3 python3-dev python3-setuptools git && \
    apt-get clean

# Clonar el repositorio de NS-3 desde GitHub
RUN git clone https://gitlab.com/nsnam/ns-3-allinone.git /root/ns-3


# Compilar NS-3
RUN cd /root/ns-3 && \
    python3 waf configure && \
    python3 waf build

# Establecer variables de entorno para NS-3
ENV NS3_HOME /root/ns-3
ENV LD_LIBRARY_PATH $NS3_HOME/build/lib

COPY . /root/ns-3

# Configurar un directorio de trabajo
WORKDIR /root/ns-3

# Establecer un punto de entrada predeterminado
CMD ["/bin/bash"]