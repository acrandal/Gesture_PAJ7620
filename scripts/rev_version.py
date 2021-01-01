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


def get_updated_version_copy(existing_version: version.Version,
                             major: int = None, minor: int = None, micro: int = None) -> version.Version:
    """get_updated_version_copy
    Generates a copy of a version.Version with the specified major, minor, micro version.
    None values leaves the version components unchanged

    inputs:
        existing_version (version.Version): The existing version to base the changes on
        major (int): The major version to write to the version copy. Unchanged if None
        minor (int): The minor version to write to the version copy. Unchanged if None
        micro (int): The micro version to write to the version copy. Unchanged if None

    outputs:
        version.Version: The version copy with modified values
    """

    # Unroll version data
    new_version = deepcopy(existing_version)
    new_version_data = new_version._version  # pylint: disable=W0212
    new_release = list(new_version_data.release)
    new_version_data = list(new_version_data)

    if major:
        new_release[0] = major
    if minor:
        new_release[1] = minor
    if micro:
        new_release[2] = micro

    new_version_data[1] = tuple(new_release)
    new_version_data = version._Version(*new_version_data)  # pylint: disable=W0212
    new_version._version = new_version_data  # pylint: disable=W0212

    return new_version


def rev_doxygen_project_number(current_version: version.Version, next_version: version.Version):
    """rev_doxygen_project_number
    Updates any version references within Doxyfile

    inputs:
        current_version (version.Version): The current version of the library as it
            appears in Doxyfile
        next_version (version.Version): The next version of the library to update Doxyfile to
    """
    with open("Doxyfile", 'r+') as doxyfile:
        content = doxyfile.read()
        new_content, num_replaced = re.subn(current_version.base_version, next_version.base_version,
                                            content, flags=re.M)
        if not num_replaced:
            print("Failed to find {} in Doxyfile to update version.".format(current_version))
            return

    with open("Doxyfile", 'w') as doxyfile:
        doxyfile.write(new_content)


def rev_library_properties_version(current_version: version.Version, next_version: version.Version):
    """rev_library_properties_version
    Updates any version references within library.properties

    inputs:
        current_version (version.Version): The current version of the library as it
            appears in library.properties
        next_version (version.Version): The next version of the library to update library.properties to
    """
    with open("library.properties", "r+") as props:
        content = props.read()
        new_content, num_replaced = re.subn(current_version.base_version,
                                            next_version.base_version,
                                            content, flags=re.M)
        if not num_replaced:
            print("Failed to find {} in library.properties to update version.".format(current_version))
            return

    with open("library.properties", "w") as props:
        props.write(new_content)


def get_examples_changed(example_dir: str, previous_version: version.Version) -> List[str]:
    """get_examples_changed
    Fetch all of the example files which have changed since the last version

    inputs:
        example_dir (str): The directory which contains the examples.
        previous_version (version.Version): The previous version of the library, used
            to check against for example changes.
    outputs:
        List[str]: The filenames (without paths) of each file in the example_dir which has changed
    """
    # Get all example files changed since current version tag
    changed_examples = subprocess.run(args=["git", "diff", "--name-only",
                                       "v{}".format(previous_version.base_version), "--", example_dir],
                                      capture_output=True,
                                      check=True)

    changed_examples_str = changed_examples.stdout.decode()
    return [os.path.basename(f) for f in changed_examples_str.split('\n') if os.path.basename(f)]


def rev_example_versions(example_dir: str, previous_version: version.Version):
    """rev_example_versions
    Update the version of each example iff the example file has changed

    inputs:
        example_dir (str): The directory which contains the examples.
        previous_version (version.Version): The previous version of the library, used
            to check against for example changes.
    """
    with open(os.path.join(example_dir, "examples.doc"), 'r+') as ex_file:
        examples = re.findall(r'/\*\*(.*?)\*/', ex_file.read(), flags=re.M | re.DOTALL)
        new_examples = []
        changed_files = get_examples_changed(example_dir, previous_version)

        print("Changed: {}".format(changed_files))

        for example in examples:
            example_filename = re.search(r'example\s+(.*\..*)\n', example)[1]
            if example_filename in changed_files:
                ex_version = version.parse(re.search(r'version\s+({})\n'.format(version.VERSION_PATTERN),
                                                     example, flags=re.VERBOSE | re.IGNORECASE | re.M)[1])
                ex_version = get_updated_version_copy(ex_version, minor=ex_version.minor + 1)
                new_example = re.sub(r'(version\s+)(.*)\n', '\g<1>{}\n'.format(ex_version.base_version),
                                     example)
                print("NEW: {}".format(example_filename))
                new_examples.append(new_example)
            else:
                print("OLD: {}".format(example_filename))
                new_examples.append(example)

        output = '\n\n'.join([r'/**{}*/'.format(e) for e in new_examples])

    with open(os.path.join(example_dir, "examples.doc"), 'w') as ex_file:
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

    return get_updated_version_copy(current_version, minor=current_version.minor + 1, micro=0)


def calculate_prev_version(current_version: version.Version) -> version.Version:
    """calculate_prev_version
    Get the previous version based upon the current one by decrementing the minor version
        if micro version is 0, else by decrementing the micro version

    inputs:
        current_version (version.Version): The current version of the library

    outputs:
        version.Version: The previous version of the library
    """

    if current_version.micro == 0:
        return get_updated_version_copy(current_version, minor=current_version.minor - 1)
    else:
        return get_updated_version_copy(current_version, micro=current_version.micro - 1)


@click.command()
@click.option("--current-version", "--current",
              help="Current version of the library. Fetched from library.properties by default")
@click.option("--previous-version", "--previous",
              help="Previous version of the library. Defaults to current version with minor version - 1 "
                   "if micro version is 0 (e.g. 1.4.0 -> 1.3.0), or micro version - 1 "
                   "(e.g. 1.4.2 -> 1.4.1)")
@click.option("--next-version", "--next",
              help="Version to rev the library to. Defaults to current version with minor version + 1 "
                   "and micro version set to 0 (e.g. 1.3.2 -> 1.4.0).")
@click.option("--example-dir", help="Path to the examples directory", default="examples")
def cmd(current_version, previous_version, next_version, example_dir):
    """Run the rev version
    """
    # Move to project root to find files relative to
    project_root = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")
    os.chdir(project_root)
    if not current_version:
        current_version = get_current_version()
        print("Found current version {}".format(current_version.base_version))

    if not previous_version:
        previous_version = calculate_prev_version(current_version=current_version)
        print("Found previous version {}".format(previous_version.base_version))

    if not next_version:
        next_version = calculate_next_version(current_version=current_version)
        print("Found next version {}".format(next_version.base_version))

    rev_doxygen_project_number(current_version=current_version, next_version=next_version)
    rev_library_properties_version(current_version=current_version, next_version=next_version)
    rev_example_versions(example_dir=example_dir, previous_version=previous_version)


if __name__ == "__main__":
    cmd()  # pylint: disable=E1120
