# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class Clamr(CMakePackage):
    """The CLAMR code is a cell-based adaptive mesh refinement (AMR)
    mini-app developed as a testbed for hybrid algorithm development
    using MPI and OpenCL GPU code.

    This version uses the instrumented version. Should ONLY be used in
    conjunction with https://github.com/CUP-ECS/irregular-benchmarking
    """

    homepage = "https://github.com/lanl/CLAMR"
    git = "https://github.com/carsonwoods/CLAMR.git"
    tags = ["proxy-app"]

    version("benchmark", branch="benchmark")

    variant(
        "graphics",
        default="opengl",
        values=("opengl", "mpe", "none"),
        description="Build with specified graphics support",
    )
    variant(
        "precision",
        default="mixed",
        values=("single", "mixed", "full"),
        description="single, mixed, or full double precision values",
    )

    depends_on("cmake@3.1:", type="build")
    depends_on("mpi")
    depends_on("mpe", when="graphics=mpe")

    def cmake_args(self):
        spec = self.spec
        cmake_args = []
        if "graphics=none" in spec:
            cmake_args.append("-DGRAPHICS_TYPE=None")
        elif "graphics=mpe" in spec:
            cmake_args.append("-DGRAPHICS_TYPE=MPE")
        else:
            cmake_args.append("-DGRAPHICS_TYPE=OpenGL")

        if "precision=full" in spec:
            cmake_args.append("-DPRECISION_TYPE=full_precision")
        elif "precision=single" in spec:
            cmake_args.append("-DPRECISION_TYPE=minimum_precision")
        else:
            cmake_args.append("-DPRECISION_TYPE=mixed_precision")

        # if MIC, then -DMIC_NATIVE=yes
        return cmake_args

    def install(self, spec, prefix):
        install("README", prefix)
        install("LICENSE", prefix)
        install_tree("docs", join_path(prefix, "docs"))
        install_tree("tests", join_path(prefix, "tests"))
        with working_dir(self.build_directory):
            make("install")
