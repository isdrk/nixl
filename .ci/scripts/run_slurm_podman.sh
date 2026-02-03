#!/bin/bash -xe

set -o pipefail

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

function usage()
{
cat <<EOF
Usage: $0 <[options=value]>
Options:
--script_path                 Path to the script to execute
--docker_image                Docker image name (optional, uses DOCKER_IMAGE_NAME env if not set)
--slurm_job_id                SLURM job ID (optional, uses SLURM_JOB_ID env if not set)
--slurm_nodes                 Number of SLURM nodes (optional, uses SLURM_NODES env if not set)
--slurm_head_node             SLURM head node (optional, uses SLURM_HEAD_NODE env if not set)
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

[ $# -eq 0 ] && usage

# Arrays to collect script arguments
SCRIPT_ARGS=()

while getopts ":h-:" optchar; do
    case "${optchar}" in
        -)
            case "${OPTARG}" in
                script_path=*)
                    script_path=${OPTARG#*=}
                    ;;
                docker_image=*)
                    docker_image=${OPTARG#*=}
                    ;;
                slurm_job_id=*)
                    slurm_job_id=${OPTARG#*=}
                    ;;
                slurm_nodes=*)
                    slurm_nodes=${OPTARG#*=}
                    ;;
                slurm_head_node=*)
                    slurm_head_node=${OPTARG#*=}
                    ;;
                ucx_version=*)
                    ucx_version=${OPTARG#*=}
                    SCRIPT_ARGS+=("--ucx_version=${OPTARG#*=}")
                    ;;
                container_tag=*)
                    container_tag=${OPTARG#*=}
                    SCRIPT_ARGS+=("--container_tag=${OPTARG#*=}")
                    ;;
                base_image=*)
                    base_image=${OPTARG#*=}
                    SCRIPT_ARGS+=("--base_image=${OPTARG#*=}")
                    ;;
                nixl_install_dir=*)
                    nixl_install_dir=${OPTARG#*=}
                    SCRIPT_ARGS+=("--nixl_install_dir=${OPTARG#*=}")
                    ;;
                nixl_build_dir=*)
                    nixl_build_dir=${OPTARG#*=}
                    SCRIPT_ARGS+=("--nixl_build_dir=${OPTARG#*=}")
                    ;;
                tag=*)
                    tag=${OPTARG#*=}
                    SCRIPT_ARGS+=("--tag=${OPTARG#*=}")
                    ;;
                dockerfile=*)
                    dockerfile=${OPTARG#*=}
                    SCRIPT_ARGS+=("--dockerfile=${OPTARG#*=}")
                    ;;
                repo_user=*)
                    repo_user=${OPTARG#*=}
                    SCRIPT_ARGS+=("--repo_user=${OPTARG#*=}")
                    ;;
                repo_pass=*)
                    repo_pass=${OPTARG#*=}
                    SCRIPT_ARGS+=("--repo_pass=${OPTARG#*=}")
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

# Required parameter
: ${script_path:?Missing --script_path}

# Use environment variables as fallback for SLURM parameters
docker_image=${docker_image:-${DOCKER_IMAGE_NAME}}
slurm_job_id=${slurm_job_id:-${SLURM_JOB_ID}}
slurm_nodes=${slurm_nodes:-${SLURM_NODES}}
slurm_head_node=${slurm_head_node:-${SLURM_HEAD_NODE}}

# Add credentials from environment if not passed as arguments
repo_user=${repo_user:-${REPO_USER}}
repo_pass=${repo_pass:-${REPO_PASS}}

# Add credentials to script args if available
[[ -n "${repo_user}" && ! " ${SCRIPT_ARGS[*]} " =~ " --repo_user=" ]] && SCRIPT_ARGS+=("--repo_user=${repo_user}")
[[ -n "${repo_pass}" && ! " ${SCRIPT_ARGS[*]} " =~ " --repo_pass=" ]] && SCRIPT_ARGS+=("--repo_pass=${repo_pass}")

# Build SLURM command using bash arrays (professional approach)
SLURM_CMD=(
    "srun"
    "--jobid=${slurm_job_id}"
    "--nodes=${slurm_nodes}"
    "--mpi=pmix"
    "--container-writable"
    "--container-remap-root"
    "--container-image=${docker_image}"
)

# Add the script path and its arguments
SLURM_CMD+=("${script_path}")
SLURM_CMD+=("${SCRIPT_ARGS[@]}")

echo "INFO: Executing script: ${script_path}"
echo "INFO: Using SLURM job ID: ${slurm_job_id}"
echo "INFO: Using Docker image: ${docker_image}"
echo "INFO: Script arguments: ${SCRIPT_ARGS[*]}"
echo "INFO: SLURM command: ${SLURM_CMD[*]}"

# Validate SLURM_HEAD_NODE is set
if [ -z "${slurm_head_node}" ]; then
    echo "ERROR: SLURM_HEAD_NODE is not set or empty"
    exit 1
fi

# Execute based on head node type
case "${slurm_head_node}" in
    scctl)
        echo "INFO: Using scctl client to connect and execute SLURM command"
        scctl --raw-errors client connect -- "${SLURM_CMD[@]}"
        ;;
    *)
        echo "ERROR: Invalid SLURM_HEAD_NODE value: ${slurm_head_node}"
        exit 1
        ;;
esac

echo "INFO: Script execution completed successfully"
