_lokatt_gettop()
{
	T=$(git rev-parse --show-toplevel)

	if [[ -z ${T} ]]; then
		echo "error: failed to get top of lokatt git"
		return 1
	fi
}

# Set T when sourcing this script, abort if we can't find top
_lokatt_gettop || return 1

_lokatt_activate_venv()
{
	if [[ ! -e ${T}/venv/bin/activate ]]; then
		echo "error: missing virtual environment" >&2
		echo "hint: run lokatt_setup" >&2
		return 1
	fi
	echo "Activate python environment"
	source ${T}/venv/bin/activate
	return $?
}

_lokatt_create_venv()
{
	echo "creating virtual environment..."
	${PYVENV} --without-pip venv
	# TODO: Revert the patch that added this workaround.
	# This is a workaround for a bug in pyvenv-3.4 in Debian and Ubuntu
	# It should be possible to use pyvenv with pip from Ubuntu 15.04
	# For more information see
	# https://gist.github.com/denilsonsa/21e50a357f2d4920091e#file-python-virtual-environments-on-debian-and-ubuntu-md
	source ${T}/venv/bin/activate || return $?
	curl https://bootstrap.pypa.io/get-pip.py | python
	res=$?
	if [[ $res != 0 ]]; then
		echo "error: failed to install pip" >&2
	fi
	deactivate
	return $res || $?
}

_lokatt_install_dependencies()
{
	echo "Ensure python dependencies"
	pip install -q -r ${T}/scripts/dependencies.txt
	return $?
}

lokatt_setup()
{
	if [ ! -d "${T}/venv" ]; then

		if [[ -z "${PYVENV}" ]]; then
			echo "error: pyvenv not found" >&2
			echo "hint: set PYVENV" >&2
			return 2
		fi

		_lokatt_create_venv || return 3
	fi
	_lokatt_activate_venv || return 4
	_lokatt_install_dependencies || return 5
}

lokatt_setup
