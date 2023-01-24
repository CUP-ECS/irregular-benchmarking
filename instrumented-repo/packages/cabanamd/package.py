# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class Cabanamd(CMakePackage):
     """CabanaMD: Molecular dynamics proxy application based on Cabana"""
 
     homepage = "https://github.com/ECP-copa/CabanaMD"
     git = "https://github.com/ECP-copa/CabanaMD.git"
 
     maintainers = ["carsonwoods"]

     version("master", branch="master")

     depends_on("cabana@master +cajita", type=("build", "link", "run"))
     depends_on("kokkos@3.4", type=("build", "link", "run"))
     depends_on("googletest@1.10.0", type=("build", "link", "run"))
     
     def cmake_args(self):
         # FIXME: Add arguments other than
         # FIXME: CMAKE_INSTALL_PREFIX and CMAKE_BUILD_TYPE
         # FIXME: If not needed delete this function
         args = []
         return args
