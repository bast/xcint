#!/usr/bin/env python

import os
import sys
from cboot.cboot import parse_options, gen_cmake_command, check_cmake_exists, setup_build_path, run_cmake

options = """
Usage:
  ./setup [options] [<builddir>]
  ./setup (-h | --help)

Options:
  --cc=<CC>               C compiler [default: gcc].
  --cxx=<CXX>             C++ compiler [default: g++].
  --omp                   Enable OpenMP (sets -DENABLE_OMP=ON).
  --mpi                   Enable MPI (sets -DENABLE_MPI=ON).
  --opencl                Enable OpenCL (sets -DENABLE_OPENCL=ON).
  --coverage              Enable code coverage (sets -DENABLE_CODE_COVERAGE=ON).
  --mkl=<MKL>             Pass MKL flag to the compiler and linker (sequential, parallel, or cluster).
  --blas=<BLAS>           Specify BLAS library (auto, builtin, none, or full path) [default: auto].
  --lapack=<LAPACK>       Specify LAPACK library (auto, builtin, none, or full path) [default: auto].
  --explicit-libs=<LIBS>  Explicit linker specification for extra libraries; passed directly to the linker.
  --type=<TYPE>           Set the CMake build type (debug, release) [default: release].
  <builddir>              Build directory.
  --show                  Show CMake command and exit. Do not write any files.
  -h --help               Show this screen.
"""

root_directory = os.path.dirname(os.path.realpath(__file__))
default_build_path = os.path.join(root_directory, 'build')

#-------------------------------------------------------------------------------

if __name__ == '__main__':

    arguments = parse_options(options)
    command = '%s %s' % (gen_cmake_command(options, arguments), root_directory)

    # check that CMake is available, if not stop
    check_cmake_exists('cmake')

    # deal with build path
    build_path = arguments['<builddir>']
    if build_path == None:
        build_path = default_build_path
    if not arguments['--show']:
        setup_build_path(build_path)

    print('%s\n' % command)
    if arguments['--show']:
        sys.exit(0)

    run_cmake(command, build_path, default_build_path)
