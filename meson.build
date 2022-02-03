###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Build script
#
# CREATED:          12/20/2021
#
# LAST EDITED:      02/03/2022
#
# Copyright 2021, Ethan D. Twardy
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
###

project(
  'libserdec', 'c',
  version: '0.1.2',
  default_options: [
    'buildtype=debug',
    'warning_level=3',
    'c_std=gnu18',
  ]
)
add_project_arguments(
  '-frecord-gcc-switches',
  language: 'c'
)
include_directories('serdec')

# Headers for installation
install_headers(
  'serdec/yaml.h',
  subdir: 'serdec',
)

# Sources for the library
libserdec_sources = files([
  'serdec/yaml.c',
])

libyaml = dependency('yaml-0.1')
libserdec = library(
  'serdec',
  sources: libserdec_sources,
  dependencies: [libyaml],
  version: meson.project_version(),
  install: true,
)

pkgconfig = import('pkgconfig')
pkgconfig.generate(libserdec, filebase: 'libserdec')

executable(
  'testserdec',
  'test/yaml.c',
  link_with: [libserdec],
)

###############################################################################
