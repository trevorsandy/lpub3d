FROM archlinux/base:latest
LABEL maintainer "Trevor SANDY <trevor.sandy@gmail.com>"
RUN pacman -Syu --noconfirm > CreateDockerImage_archlinux.log 2>&1
RUN pacman -S --noconfirm --needed git wget unzip ccache base-devel >> CreateDockerImage_archlinux.log 2>&1
RUN pacman -S --noconfirm --needed xorg-server-xvfb desktop-file-utils >> CreateDockerImage_archlinux.log 2>&1
RUN pacman -S --noconfirm --needed qt5-base qt5-tools >> CreateDockerImage_archlinux.log 2>&1
RUN pacman -S --noconfirm --needed sudo \
    && groupadd -r user -g 1000 \
    && useradd -u 1000 -r -g user -m -d /user -s /sbin/nologin -c "Build pkg user" user \
    && chmod 755 /user \
    && echo "user ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/user \
    && chmod 0440 /etc/sudoers.d/user
WORKDIR /user
USER user
VOLUME ["/buildpkg", "/user/pkgbuild/src/lpub3d_linux_3rdparty"]
CMD sudo chown -R user:user /user/pkgbuild/.* \
    && wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreatePkg.sh \
    && export DOCKER=true \
    && chmod a+x CreatePkg.sh \
    && ./CreatePkg.sh \
    && if test -d /buildpkg; then \
      sudo cp -f /user/pkgbuild/*.pkg.tar.xz /buildpkg/; \
      sudo cp -f /user/pkgbuild/src/*.log /buildpkg/; \
      sudo cp -f /user/*.log /buildpkg/; \
      sudo cp -f /*.log /buildpkg/; \
    fi
