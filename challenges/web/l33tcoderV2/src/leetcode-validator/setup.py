from setuptools import setup, find_packages

setup(
    name="uscg_leetcode_validator_2",
    version="1.0.0",
    packages=find_packages(),
    author="Jacob Elliott",
    author_email="coachelliott@uscybergames.org",
    description="Used to validate restricted python code inputs against test cases.",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
    entry_points={
        'console_scripts': [
            'uscg-leetcode-validator=uscg_leetcode_validator.main:main',
        ],
    },
)
