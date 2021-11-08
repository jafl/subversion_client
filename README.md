[![Github CI](https://github.com/jafl/subversion_client/actions/workflows/ci.yml/badge.svg)](https://github.com/jafl/subversion_client/actions/workflows/ci.yml)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_subversion_client&metric=alert_status)](https://sonarcloud.io/dashboard?id=jafl_subversion_client&branch=main)

[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_subversion_client&metric=sqale_rating)](https://sonarcloud.io/dashboard?id=jafl_subversion_client&branch=main)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_subversion_client&metric=security_rating)](https://sonarcloud.io/dashboard?id=jafl_subversion_client&branch=main)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_subversion_client&metric=vulnerabilities)](https://sonarcloud.io/dashboard?id=jafl_subversion_client&branch=main)

[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?branch=main&project=jafl_subversion_client&metric=ncloc)](https://sonarcloud.io/dashboard?id=jafl_subversion_client&branch=main)

# Subversion Client

This X application provides a simple interface for performing common subversion tasks, e.g., updating from the repository and checking in to the repository.  The [on-line help](http://nps-svnclient.sourceforge.net/help.html) explains all the features of the program.


## Building from source

1. Install the [JX Application Framework](https://github.com/jafl/jx_application_framework),  and set the `JX_ROOT` environment variable to point to its `include` directory.
1. `makemake`
1. `make`


## Installing a binary

For macOS, the easiest way to install is via [Homebrew](https://brew.sh):

    brew install --cask xquartz
    brew tap jafl/jx
    brew install jafl/jx/nps_svn_client

For all other systems, download a package from:

* https://github.com/jafl/subversion_client/releases
* http://sourceforge.net/p/nps-svnclient

If you download the tar, unpack it, cd into the directory, and run `sudo ./install`.

Otherwise, put the program (`nps_svn_client`) in a directory that is on your execution path.  `~/bin` is typically good choices.

### Requirements

On MacOS, this program requires XQuartz, the X11 server.  Before running this program, you must first start XQuartz.


## FAQ

For general questions, refer to the [Programs FAQ](https://github.com/jafl/jx_application_framework/blob/master/APPS.md).
