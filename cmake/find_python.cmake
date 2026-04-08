set(VENV_PYTHON "${CMAKE_SOURCE_DIR}/venv/bin/python")

if (EXISTS "${VENV_PYTHON}")
    message(STATUS "Using Python from virtual environment: ${VENV_PYTHON}")
    set(Python3_EXECUTABLE "${VENV_PYTHON}" CACHE FILEPATH "Preferred Python interpreter")
else ()
    message(STATUS "Virtual environment not found. Falling back to system Python.")
endif ()
