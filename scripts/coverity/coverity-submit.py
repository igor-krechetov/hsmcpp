#!/usr/bin/env python
#
# coverity-submit - submit project to Coverity for scanning
#
# By Eric S. Raymond, May 2012.
# SPDX-License-Identifier: BSD-2-Clause
#
# Version 2.0 by Igor Krechetov, Feb 2023
# Improved to be used inside GitHub actions.
#
# This code runs under both Python 2 and Python 3. Preserve this property!
from __future__ import print_function

import os, pwd, sys, stat
import tempfile, shutil, datetime, subprocess
import argparse

version = "2.0"


def do_or_die(cmd):
    if verbose:
        print(cmd)
    if not dryrun:
        if os.system(cmd) != 0:
            sys.stderr.write("Command failed.\n")
            sys.exit(1)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='coverity-submit - submit project to Coverity for scanning')

    parser.add_argument('-user', '-u', type=str, required=True, help='build version')
    parser.add_argument('-build_version', '-b', type=str, required=True, help='build version')
    parser.add_argument('-dryrun', action="store_true", required=False, default=False, help='dry run of all commands')
    parser.add_argument('-description', '-t', type=str, required=True, help='project description')
    parser.add_argument('-verbose', '-v', action="store_true", required=False, default=False, help='verbose execution')
    parser.add_argument('-project', '-p', type=str, required=True, help='project name')
    parser.add_argument('-project_url', '-pu', type=str, required=False, help='project url name')
    parser.add_argument('-token', type=str, required=True, help='Coverity token')
    parser.add_argument('-email', type=str, required=True, help='user email')
    parser.add_argument('-prebuild', type=str, required=False, help='prebuild command')
    parser.add_argument('-build', type=str, required=True, help='build command')
    parser.add_argument('-postbuild', type=str, required=False, help='post-build command')
    parser.add_argument('-config', type=str, required=True, help='coverity config')

    args = parser.parse_args()
    name = args.user
    build_version = args.build_version
    dryrun = args.dryrun
    description = args.description
    verbose = args.verbose

    covname = args.project
    covname_url = args.project_url
    if covname_url is None:
        covname_url = covname
    token = args.token
    email = args.email
    prebuild = args.prebuild
    build = args.build
    postbuild = args.postbuild
    cov_config = args.config

    # Announce self
    print("coverity-submit version %s..." % version)

    # Work around a known bug in environment restoration under cov-build.
    # Without this, xmlto won't run.
    os.environ["XML_CATALOG_FILES"] = '/etc/xml/catalog'

    # Build local stuff
    print("Rebuilding and scanning...")
    if prebuild:
        do_or_die(prebuild)
    do_or_die("rm -fr cov-int && cov-build --config " + cov_config + " --dir cov-int " + build)
    if postbuild:
        do_or_die(postbuild)

    # Create the tarball
    if verbose:
        print("Bundling up required metadata...")
        
    readme = """\
    Name: %(name)s
    Email: %(email)s
    Project: %(covname)s
    Build-Version: %(build_version)s
    Description: %(description)s
    Submitted-by: coverity-submit %(version)s
    """ % globals()

    if verbose:
        sys.stdout.write(readme)

    tmpdir = tempfile.mkdtemp()

    if not dryrun:
        with open(os.path.join(tmpdir, "README"), "w") as wfp:
            wfp.write(readme)

    tarball = "%s-scan.tgz" % covname
    tarball = tarball.replace('/', '-')

    if verbose and not dryrun:
        shutil.copy("cov-int/build-log.txt", "build-log.txt")

    do_or_die("mv cov-int %s; (cd %s; tar -czf %s README cov-int; rm -fr README cov-int)" % (tmpdir, tmpdir, tarball))
    print("Posting the analysis request...")
    do_or_die('''curl \
    --form file=@%(tmpdir)s/%(tarball)s \
    --form project="%(covname)s" \
    --form token=%(token)s \
    --form email=%(email)s \
    --form version="%(build_version)s" \
    --form description="%(description)s" \
      https://scan.coverity.com/builds?project=%(covname_url)s \
     ''' % globals())

    try:
        os.remove(os.path.join(tmpdir, tarball))
        os.rmdir(tmpdir)
    except OSError:
        pass

    print("Finished")

# The following sets edit modes for GNU EMACS
# Local Variables:
# mode:python
# End:
