# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *


class Cabanamd(CMakePackage):
    """CabanaMD: Molecular dynamics proxy application based on Cabana
    Warning: this package's variants do not work.
             This might be fixed later, or it might not. Do not use them unless you're
             willing to fix them and the dependency issues.
    """

    homepage = "https://github.com/ECP-copa/CabanaMD"
    git = "https://github.com/ECP-copa/CabanaMD.git"

    maintainers = ["carsonwoods"]

    version("master", branch="master")

    variant("nnp", default=False, description="Enable neural network potential")
    variant("cuda", default=False, description="Enable CUDA support")

    depends_on("cabana@instrumented +cajita", type=("build", "link", "run"))
    depends_on("kokkos@3.4 +cuda +wrapper", type=("build", "link", "run"), when="+cuda")
    depends_on("kokkos@3.4", type=("build", "link", "run"), when="~cuda")
    depends_on("googletest@1.10.0", type=("build", "link", "run"))
    depends_on("n2p2@2.0.1", type=("build", "link", "run"), when="+nnp")

    def cmake_args(self):
        spec = self.spec
        args = []

        if "+nnp" in self.spec:
            args.extend(
                [
                    "-D N2P2_DIR={0}".format(spec["n2p2"].prefix),
                    "-D CabanaMD_ENABLE_NNP=ON",
                    "-D CabanaMD_VECTORLENGTH_NNP=1",
                    "-D CabanaMD_MAXSYMMFUNC_NNP=30",
                ]
            )

        if "+cuda" in self.spec:
            args.append(
                "-D CMAKE_CXX_COMPILER={0}/bin/nvcc_wrapper".format(
                    spec["kokkos"].prefix
                )
            )

        return args
