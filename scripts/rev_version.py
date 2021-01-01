#!/usr/bin/env python
"""Rev Version
Updates the version of the library in the following files

  - Doxyfile              # Doxygen
  - library.properties    # Arduino

Also updates the version of any examples which have been updated
since the last version (in <path to examples>/examples.doc)
"""

import os
import re
import subprocess

from copy import deepcopy
from typing import List
from packaging import version

import click


def rev_doxygen_project_number(current_version: version.Version, new_version: version.Version):
    """rev_doxygen_project_number
    Updates any version references within Doxyfile

    inputs:
        current_version (version.Version): The current version of the library as it
            appears in Doxyfile
        new_version (version.Version): The next version of the library to update Doxyfile to
    """
    with open("Doxyfile", 'r+') as doxyfile:
        content = doxyfile.read()
        new_content, num_replaced = re.subn(current_version, new_version, content, flags=re.M)
        if not num_replaced:
            print("Failed to find {} in Doxyfile to update version.".format(current_version))
        else:
            doxyfile.write(new_content)


def rev_library_properties_version(current_version: version.Version, new_version: version.Version):
    """rev_library_properties_version
    Updates any version references within library.properties

    inputs:
        current_version (version.Version): The current version of the library as it
            appears in library.properties
        new_version (version.Version): The next version of the library to update library.properties to
    """
    with open("library.properties", "r+") as props:
        content = props.read()
        new_content, num_replaced = re.subn(current_version.base_version,
                                            new_version.base_version,
                                            content, flags=re.M)
        if not num_replaced:
            print("Failed to find {} in library.properties to update version.".format(current_version))
        else:
            props.write(new_content)


def get_examples_changed(example_dir: str, current_version: version.Version) -> List[str]:
    """get_examples_changed
    Fetch all of the example files which have changed since the last version

    inputs:
        example_dir (str): The directory which contains the examples.
        current_version (version.Version): The current version of the library, used
            to check against for example changes.
    outputs:
        List[str]: The filenames (without paths) of each file in the example_dir which has changed
    """
    # Get all example files changed since current version tag
    changed_examples = subprocess.run(["git", "diff", "--name-only",
                                       current_version.base_version, "--", example_dir],
                                      capture_output=True,
                                      shell=True, check=True)
    return [os.path.basename(f) for f in changed_examples.stdout.split('\n')]


def rev_example_versions(example_dir: str, current_version: version.Version):
    """rev_example_versions
    Update the version of each example iff the example file has changed

    inputs:
        example_dir (str): The directory which contains the examples.
        current_version (version.Version): The current version of the library, used
            to check against for example changes.
    """
    with open(os.path.join(example_dir, "examples.doc"), 'r+') as ex_file:
        examples = re.findall(r'/\*\*(.*)\*/', ex_file.read())
        new_examples = []
        changed_files = get_examples_changed(example_dir, current_version)

        for example in examples:
            example_filename = re.search(r'example\s+(.*\..*)\n', example)
            if example_filename in changed_files:
                ex_version = version.parse(re.search(r'version\s+({})\n'.format(version.VERSION_PATTERN),
                                                     example, flags=re.VERBOSE | re.IGNORECASE)[1])
                ex_version.minor += 1
                new_example = re.sub(r'(version\s+)(.*)\n', '\1{}'.format(ex_version.base_version),
                                     example)
                new_examples.append(new_example)

        output = '\n\n'.join([r'/\*\*{}\*/'.format(e) for e in new_examples])
        ex_file.write(output)


def get_current_version() -> version.Version:
    """get_current_version
    Gets the current version as it exists in library.properties

    outputs:
        version.Version: The version as it exists in library.properties
    """
    with open("library.properties", 'r') as props:
        content = props.read()
        existing_version = version.parse(re.search("version=({})".format(version.VERSION_PATTERN),
                                                   content, flags=re.VERBOSE | re.IGNORECASE)[1])
    return existing_version


def calculate_next_version(current_version: version.Version) -> version.Version:
    """calculate_next_version
    Get the next version based upon the current one by incrementing the minor version
        and setting the micro version to 0

    inputs:
        current_version (version.Version): The current version of the library

    outputs:
        version.Version: The next version of the library to rev to
    """
    new_version = deepcopy(current_version)
    new_version.minor += 1
    new_version.micro = 0

    return new_version


@click.command()
@click.option("--current-version", "--current",
              help="Current version of the library. Fetched from library.properties by default")
@click.option("--next-version", "--next",
              help="Version to rev the library to. Defaults to current version with minor version + 1 "
                   "and micro version set to 0 (e.g. 1.3.2 -> 1.4.0).")
@click.option("--example-dir", help="Path to the examples directory", default="examples")
def cmd(current_version, next_version, example_dir):
    """Run the rev version
    """
    # Move to project root to find files relative to
    project_root = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")
    os.chdir(project_root)
    if not current_version:
        current_version = get_current_version()
        print("Found current version {}".format(current_version.base_version))

    if not next_version:
        next_version = calculate_next_version(current_version=current_version)
        print("Found next version {}".format(next_version.base_version))

    rev_example_versions(example_dir=example_dir, current_version=current_version)


if __name__ == "__main__":
    cmd()  # pylint: disable=E1120
