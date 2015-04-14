#!/bin/sh

if [ $# -lt 3 ]; then
	echo "usage ./build_rpm.sh name version source_dir"
	echo "   name:       package name"
	echo "   version:    package version"
	echo "   source_dir: source code dir root"
	exit 
fi

_topdir=~/rpm
echo %_topdir ${_topdir} > ~/.rpmmacros	
mkdir -p ${_topdir}/SOURCES
mkdir -p ${_topdir}/SPECS
mkdir -p ${_topdir}/BUILD
mkdir -p ${_topdir}/RPMS
mkdir -p ${_topdir}/RPMS/i386
mkdir -p ${_topdir}/SRPMS

name=$1
version=$2
source_dir=$3

spec_file=deploy/dispatcher.spec

package_dir=${name}-${version}

cd $source_dir
svn up
svn_rev=`svn info |grep "Last Changed Rev"|cut -d ' ' -f 4`
release=${svn_rev}
echo $release
mkdir -p ../${package_dir}
cp -rf * ../${package_dir}/

tar czf ${_topdir}/SOURCES/${package_dir}.tar.gz ../${package_dir}  --exclude=.svn* 
rm -fr ../${package_dir}

sed "s/^Version:.*$/Version: ${version}/g" $spec_file |
sed "s/^Release:.*$/Release: ${release}/g" |
sed "s/^Name:.*$/Name: ${name}/g" > tmp.spec

rpmbuild -bb tmp.spec
rm tmp.spec
