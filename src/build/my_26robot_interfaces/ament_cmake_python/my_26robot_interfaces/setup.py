from setuptools import find_packages
from setuptools import setup

setup(
    name='my_26robot_interfaces',
    version='0.0.0',
    packages=find_packages(
        include=('my_26robot_interfaces', 'my_26robot_interfaces.*')),
)
