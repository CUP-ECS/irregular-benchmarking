# Copyright 2013-2023 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack.package import *
from spack.pkg.builtin.kokkos import Kokkos


class Cabana(CMakePackage):
    """
    The Exascale Co-Design Center for Particle Applications Toolkit


    This version uses the instrumented version. Should ONLY be used in
    conjunction with https://github.com/CUP-ECS/irregular-benchmarking
    """

    homepage = "https://github.com/ECP-copa/Cabana"
    git = "https://github.com/CUP-ECS/Cabana.git"

    maintainers = ["carsonwoods"]

    tags = ["e4s", "ecp"]

    version("instrumented", branch="instrumented")

    _kokkos_backends = Kokkos.devices_variants
    for _backend in _kokkos_backends:
        _deflt, _descr = _kokkos_backends[_backend]
        variant(_backend.lower(), default=_deflt, description=_descr)

    variant("shared", default=True, description="Build shared libraries")
    variant("mpi", default=True, description="Build with mpi support")
    variant("arborx", default=False, description="Build with ArborX support")
    variant("heffte", default=False, description="Build with heFFTe support")
    variant("hypre", default=False, description="Build with HYPRE support")
    variant("cajita", default=False, description="Build Cajita subpackage")
    variant("testing", default=False, description="Build unit tests")
    variant("examples", default=False, description="Build tutorial examples")
    variant("performance_testing", default=False, description="Build performance tests")

    depends_on("cmake@3.9:", type="build", when="@:0.4.0")
    depends_on("cmake@3.16:", type="build", when="@0.5.0:")
    depends_on("googletest", type="build", when="testing")
    _versions = {":0.2": "-legacy", "0.3:": "@3.1:", "0.4:": "@3.2:", "master": "@3.4:", "instrumented": "@3.2"}
    for _version in _versions:
        _kk_version = _versions[_version]
        for _backend in _kokkos_backends:
            if _kk_version == "-legacy" and _backend == "pthread":
                _kk_spec = "kokkos-legacy+pthreads"
            elif _kk_version == "-legacy" and _backend not in ["serial", "openmp", "cuda"]:
                continue
            else:
                _kk_spec = "kokkos{0}+{1}".format(_kk_version, _backend)
            depends_on(_kk_spec, when="@{0}+{1}".format(_version, _backend))
    depends_on("arborx", when="@0.3.0:+arborx")
    depends_on("hypre-cmake@2.22.0:", when="@0.4.0:+hypre")
    depends_on("hypre-cmake@2.22.1:", when="@0.5.0:+hypre")
    # Heffte pinned at 2.x.0 because its cmakefiles can't roll forward
    # compatibilty to later minor versions.
    depends_on("heffte@2.0.0", when="@0.4.0+heffte")
    depends_on("heffte@2.1.0", when="@0.5.0:+heffte")
    depends_on("mpi", when="+mpi")

    conflicts("+cajita ~mpi")

    conflicts("+rocm", when="@:0.2.0")
    conflicts("+sycl", when="@:0.3.0")

    def cmake_args(self):
        options = [self.define_from_variant("BUILD_SHARED_LIBS", "shared")]

        enable = ["CAJITA", "TESTING", "EXAMPLES", "PERFORMANCE_TESTING"]
        require = ["ARBORX", "HEFFTE", "HYPRE"]

        # These variables were removed in 0.3.0 (where backends are
        # automatically used from Kokkos)
        if self.spec.satisfies("@:0.2.0"):
            enable += ["Serial", "OpenMP", "Cuda"]
        # MPI was changed from ENABLE to REQUIRE in 0.4.0
        if self.spec.satisfies("@:0.3.0"):
            enable += ["MPI"]
        else:
            require += ["MPI"]

        for category, cname in zip([enable, require], ["ENABLE", "REQUIRE"]):
            for var in category:
                cbn_option = "Cabana_{0}_{1}".format(cname, var)
                options.append(self.define_from_variant(cbn_option, var.lower()))

        return options
