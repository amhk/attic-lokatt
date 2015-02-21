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
	${PYVENV} ${T}/venv
	return $?
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
