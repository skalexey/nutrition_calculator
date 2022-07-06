import shutil
import os
import sys

buildPrefix = "Build-cmake"
buildConfig = "Debug"

deps = "dependencies"

def systemRemoveDir(path):
    code = -1
    if os.name == 'nt':
        code = os.system("rm \"" + path + "\"")
    else:
        code = os.system("rm -rf \"" + path + "\"")
    if code == 0:
        print("Directory '%s' removed" % path)
    else:
        print("Return code: %d" % code)

def removeDir(path):
    if os.path.exists(path):
        if not os.access(path, os.W_OK):
            os.chmod(path, stat.S_IWUSR)
        try:
            shutil.rmtree(path)
            print("Directory '%s' removed" % path)
        except OSError as e:
            print("Exception while removing a directory '%s': %s: '%s'" % (path, e.strerror, e.filename))
            print("Try to use shell command")
            systemRemoveDir(path)

    else:
        print("'%s' directory already removed" % path)

for a in sys.argv:
    if a == "release":
        buildConfig = "Release"
    if a == "g++":
        buildPrefix="Build-g++"
    if a == "full":
    	   buildConfig = ""

build = buildPrefix + "/" + buildConfig

removeDir(build)

if len(sys.argv) > 1 and sys.argv[1] == "deps":
    removeDir(deps)

