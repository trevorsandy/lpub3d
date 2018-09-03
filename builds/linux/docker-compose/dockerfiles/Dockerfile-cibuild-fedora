FROM fedora:26
LABEL maintainer "Trevor SANDY <trevor.sandy@gmail.com>"
RUN dnf install -y git wget unzip which rpmlint ccache dnf-plugins-core rpm-build > CreateDockerImage_fedora.log 2>&1
RUN dnf install -y xorg-x11-server-Xvfb desktop-file-utils >> CreateDockerImage_fedora.log 2>&1
RUN dnf install -y qt5-qtbase-devel qt5-qttools-devel >> CreateDockerImage_fedora.log 2>&1
RUN dnf update -x kernel* -y >> CreateDockerImage_fedora.log 2>&1
RUN dnf install -y sudo \
    && groupadd -r user -g 1000 \
    && useradd -u 1000 -r -g user -m -d /user -s /sbin/nologin -c "Build pkg user" user \
    && chmod 755 /user \
    && echo "user ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/user \
    && chmod 0440 /etc/sudoers.d/user
WORKDIR /user
USER user
VOLUME ["/buildpkg", "/user/rpmbuild/BUILD/lpub3d_linux_3rdparty"]
CMD sudo chown -R user:user /user/rpmbuild/.* \
    && wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreateRpm.sh \
    && wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/obs/lpub3d-rpmlintrc \
    && sudo cp -f lpub3d-rpmlintrc /etc/rpmlint \
    && export DOCKER=true \
    && chmod a+x CreateRpm.sh \
    && ./CreateRpm.sh \
    && if test -d /buildpkg; then \
      sudo cp -f /user/rpmbuild/RPMS/`uname -m`/*.rpm /buildpkg/; \
      sudo cp -f /user/rpmbuild/BUILD/*.log /buildpkg/; \
      sudo cp -f /user/*.log /buildpkg/; \
      sudo cp -f /*.log /buildpkg/; \
    fi
