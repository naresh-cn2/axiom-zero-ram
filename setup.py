from setuptools import setup, Extension

# V1.5: Now including Math and Threading support
axiom_extension = Extension(
    'libaxiom',
    sources=['axiom.c'],
    extra_compile_args=['-O3', '-march=native', '-fPIC'],
    libraries=['pthread', 'm']  # 'pthread' for threads, 'm' for math (fabs)
)

setup(
    name='axiom_core',
    version='1.5',
    description='Axiom Analyst: Multi-threaded Filtered Ingestion',
    ext_modules=[axiom_extension],
)

