#!/usr/bin/env python

import os
import re

import click

from copy import deepcopy
from packaging import version

def rev_doxygen_project_number(current_version, new_version):
    with open("Doxyfile", 'rw') as f:
        content = f.read()
        new_content, num_replaced = re.subn(current_version, new_version, content, flags = re.M)
        if not num_replaced:
            print("Failed to find {} in Doxyfile to update version.".format(current_version))
        else:
            f.write(new_content)


def rev_library_properties_version(current_version, new_version):
    with open("library.properties", "rw") as f:
        content = f.read()
        new_content, num_replaced = re.subn(current_version.base_version,
                                            new_version.base_version,
                                            content, flags = re.M)
        if not num_replaced:
            print("Failed to find {} in library.properties to update version.".format(current_version))
        else:
            f.write(new_content)


def get_examples_changed(example_dir, current_version):
    # Get all example files changed since current version tag
    import subprocess
    changed_examples = subprocess.run(["git", "diff", "--name-only",
                                       current_version.base_version, "--", example_dir],
                                      capture_output=True,
                                      shell=True, check=True)
    return [os.path.basename(f) for f in changed_examples.stdout.split('\n')]


def rev_example_versions(example_dir, current_version):
    with open(os.path.join(example_dir, "examples.doc"), "wr") as f:
        examples = re.findall(r'/\*\*(.*)\*/', f.read())
        new_examples = []
        for example in examples if example in get_examples_changed(example_dir, current_version):
            example_filename = re.search('example\s+(.*\..*)\n', example)
            if example_was_updated(example_filename):
                ex_version = version.parse(re.search('version\s+({})\n'.format(version.VERSION_PATTERN),
                                                     example, flags=re.VERBOSE|re.IGNORECASE)[1])
                ex_version.minor += 1
                new_example = re.sub('(version\s+)(.*)\n', '\1{}'.format(ex_version.base_version))
                new_examples.append(new_example)

        output = '\n\n'.join(['/\*\*{}\*/'.format(e) for e in new_examples])
        f.write(output)


def get_current_version() -> version.Version:
    with open("library.properties", 'r') as f:
        content = f.read()
        existing_version = version.parse(re.search("version=({})".format(version.VERSION_PATTERN),
                                                   content, flags=re.VERBOSE|re.IGNORECASE)[1])
    return existing_version

def calculate_next_version(current_version: version.Version) -> version.Version:
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
    # Move to project root to find files relative to
    project_root = os.path.join(os.path.dirname(os.path.realpath(__file_)), "..")
    os.chdir(project_root)
    if not current_version:
        current_version = get_current_version()
        print("Found current version {}".format(current_version.base_version))

    if not next_version:
        next_version = calculate_next_version(current_version=current_version)
        print("Found next version {}".format(next_version.base_version))

    rev_example_versions(example_dir=example_dir)


if __name__ == "__main__":
    cmd()
