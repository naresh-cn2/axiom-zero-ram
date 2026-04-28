from setuptools import setup, Extension

hydra_extension = Extension(
    'libhydra', # New internal name
    sources=['hydra_core.c'], # New file source
    extra_compile_args=['-O3', '-march=native', '-fPIC'],
    libraries=['pthread', 'm']
)

setup(
    name='hydra_core', # Professional brand
    version='1.5',
    description='HydraCore: High-Performance Multi-threaded Ingestion',
    ext_modules=[hydra_extension],
)
