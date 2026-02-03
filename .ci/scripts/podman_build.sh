#!/bin/bash -xe

set -o pipefail

function usage()
{
cat <<EOF
Usage: $0 <[options=value]>
Options:
--ucx_version                 UCX version
--container_tag               Container tag
--base_image                  Base Docker image
--nixl_install_dir            NIXL installation directory
--nixl_build_dir              NIXL build directory
--tag                         Container tag for build
--dockerfile                  Dockerfile path
--repo_user                   Repository username (optional, uses REPO_USER env if not set)
--repo_pass                   Repository password (optional, uses REPO_PASS env if not set)
EOF
exit 1
}

while getopts ":h-:" optchar; do
    case "${optchar}" in
        -)
            case "${OPTARG}" in
                ucx_version=*)
                    ucx_version=${OPTARG#*=}
                    ;;
                container_tag=*)
                    container_tag=${OPTARG#*=}
                    ;;
                base_image=*)
                    base_image=${OPTARG#*=}
                    ;;
                nixl_install_dir=*)
                    nixl_install_dir=${OPTARG#*=}
                    ;;
                nixl_build_dir=*)
                    nixl_build_dir=${OPTARG#*=}
                    ;;
                tag=*)
                    tag=${OPTARG#*=}
                    ;;
                dockerfile=*)
                    dockerfile=${OPTARG#*=}
                    ;;
                repo_user=*)
                    repo_user=${OPTARG#*=}
                    ;;
                repo_pass=*)
                    repo_pass=${OPTARG#*=}
                    ;;
                *)
                    if [ "$OPTERR" = 1 ] && [ "${optspec:0:1}" != ":" ]; then
                        echo "Unknown option --${OPTARG}" >&2
                        exit 1
                    fi
                    ;;
            esac;;
        h | *)
            usage
            exit 0
            ;;
    esac
done

# Use environment variables as fallback
repo_user=${repo_user:-${REPO_USER}}
repo_pass=${repo_pass:-${REPO_PASS}}

# Validate required parameters
: ${ucx_version:?Missing --ucx_version}
: ${container_tag:?Missing --container_tag}
: ${base_image:?Missing --base_image}
: ${nixl_install_dir:?Missing --nixl_install_dir}
: ${nixl_build_dir:?Missing --nixl_build_dir}
: ${tag:?Missing --tag}
: ${dockerfile:?Missing --dockerfile}
: ${repo_user:?Missing --repo_user or REPO_USER}
: ${repo_pass:?Missing --repo_pass or REPO_PASS}

echo "INFO: Building container with UCX version: ${ucx_version}"
echo "INFO: Container tag: ${container_tag}"
echo "INFO: Base image: ${base_image}"
echo "INFO: Build tag: ${tag}"
echo "INFO: Dockerfile: ${dockerfile}"

# Clean up storage configuration
rm -rf /etc/containers/storage.conf && rm -f /usr/share/containers/storage.conf

# Build podman command using bash arrays
PODMAN_BUILD_ARGS=(
    "podman" "build"
    "--network" "host"
    "--cgroup-manager" "cgroupfs"
    "--build-arg" "UCX_VERSION=${ucx_version}"
    "--build-arg" "PRE_INSTALLED_ENV=true"
    "--build-arg" "NIXL_INSTALL_DIR=${nixl_install_dir}"
    "--build-arg" "NIXL_BUILD_DIR=${nixl_build_dir}"
    "--build-arg" "BASE_IMAGE=${base_image}"
    "--tag" "${tag}"
    "-f" "${dockerfile}"
    "."
)

echo "INFO: Executing podman build"
"${PODMAN_BUILD_ARGS[@]}"

echo "INFO: Pushing container to registry"
podman push --creds "${repo_user}:${repo_pass}" "${tag}"

echo "INFO: Container build and push completed successfully"
