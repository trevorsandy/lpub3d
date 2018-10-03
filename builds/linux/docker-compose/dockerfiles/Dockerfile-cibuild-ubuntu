FROM ubuntu:xenial
ARG DEBIAN_FRONTEND=noninteractive
LABEL maintainer "Trevor SANDY <trevor.sandy@gmail.com>"
RUN apt-get update > CreateDockerImage_xenial.log 2>&1
RUN apt-get install -y apt-utils git wget unzip lintian build-essential debhelper fakeroot ccache lsb-release >> CreateDockerImage_xenial.log 2>&1
RUN apt-get install -y autotools-dev autoconf pkg-config libtool curl >> CreateDockerImage_xenial.log 2>&1
RUN apt-get install -y xvfb desktop-file-utils >> CreateDockerImage_xenial.log 2>&1
RUN apt-get install -y sudo \
    && groupadd -r user -g 1000 \
    && useradd -u 1000 -r -g user -m -d /user -s /sbin/nologin -c "Build pkg user" user \
    && chmod 755 /user \
    && echo "user ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/user \
    && chmod 0440 /etc/sudoers.d/user
WORKDIR /user
USER user
VOLUME ["/buildpkg", "/user/debbuild/lpub3d_linux_3rdparty"]
CMD sudo chown -R user:user /user/debbuild/.* \
    && wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreateDeb.sh \
    && export DOCKER=true \
    && chmod a+x CreateDeb.sh \
    && ./CreateDeb.sh \
    && if test -d /buildpkg; then \
      sudo cp -f /user/debbuild/*.deb /buildpkg/; \
      sudo cp -f /user/debbuild/*.dsc /buildpkg/; \
      sudo cp -f /user/debbuild/*.changes /buildpkg/; \
      sudo cp -f /user/debbuild/*log /buildpkg/; \
      sudo cp -f /user/*.log /buildpkg/; \
      sudo cp -f /*.log /buildpkg/; \
    fi
