#!/usr/bin/python3

import argparse, os, subprocess
from time import perf_counter

def timeit(time_bd, repeat, cmd, path):
  for _ in range(repeat):
    t = perf_counter()
    subprocess.Popen(cmd, shell = True, cwd=path, stdout = subprocess.DEVNULL).wait(2*time_bd)
    dt = perf_counter() - t
    print(cmd, ":", "{:.2f}".format(dt), "( time_bd =", time_bd, ")")
    if dt < time_bd:
      return True
  return False

def check_runtime(l, t, d, path, time_bd, repeat):
  if not timeit(time_bd, repeat, "./newton -l{} -t{} {}".format(l,t,d), path):
    return False

  return True


def check_runtimes(path):
  return (   check_runtime(1000, 1, 1, path, 1.01, 100)
         and check_runtime(1000, 1, 2, path, 1.48, 100)
         and check_runtime(1000, 1, 5, path, 1.52, 100)
         and check_runtime(1000, 1, 7, path, 1.64, 100)

         and check_runtime(1000, 1, 5, path, 1.52, 100)
         and check_runtime(1000, 2, 5, path, 0.70, 100)
         and check_runtime(1000, 3, 5, path, 0.55, 100)
         and check_runtime(1000, 4, 5, path, 0.42, 100)

         #and check_runtime(1000, 10, 7, path, 0.26, 100)
         #and check_runtime(50000, 10, 7, path, 594, 100)
         )


passed = True

subprocess.run(["mkdir", "-p", "extracted"])
subprocess.run(["mkdir", "-p", "reports"])
subprocess.run(["mkdir", "-p", "pictures"])


parser = argparse.ArgumentParser()
parser.add_argument("tarfile")
args = parser.parse_args()

tar_file = args.tarfile
assert( tar_file.endswith(".tar.gz") )

stem = tar_file[:-7]
while stem.find("/") != -1:
  stem = stem[stem.find("/")+1:]

extraction_path = "extracted/" + stem


# extract
print( "extracting..." )
subprocess.run(["mkdir", extraction_path])
subprocess.run(["tar", "xf", tar_file, "-C", extraction_path])

# build report
print( "building report..." )
subprocess.run(["kramdown", extraction_path + "/report.md", "-ohtml"], stdout=open("reports/" + stem + ".html","w"))

# check files
is_valid_file = lambda f: ( 
     f in ["makefile", "Makefile"] or
     f == "report.md" or
     f.endswith(".cc") or f.endswith(".c") or
     f.endswith(".hh") or f.endswith(".h") )

print( "checking for additional files..." )
if not all("material" in root or all(map(is_valid_file, files)) for (root, _, files) in os.walk(extraction_path)):
  print("ADDITIONAL FILES IN TAR")
  passed = False

# build clean build
print( "bulding and cleaning..." )
subprocess.Popen(["make", "newton"], cwd=extraction_path).wait()
subprocess.Popen(["make", "clean"], cwd=extraction_path).wait()
if not all("material" in root or all(map(is_valid_file, files)) for (root, _, files) in os.walk(extraction_path)):
  print("ADDITIONAL FILES AFTER BUILD CLEAN")
  passed = False
subprocess.Popen(["make", "newton"], cwd=extraction_path, stdout=subprocess.DEVNULL).wait()

# check times
print( "checking runtimes..." )
passed = passed and check_runtimes(extraction_path)


# copy pictures
print( "creating test picture..." )
subprocess.run([extraction_path + "/newton", "-l1000", "-t4", "7"])
subprocess.run(["mv", "newton_attractors_x7.ppm", "pictures/" + stem + "_attractors.ppm"])
subprocess.run(["mv", "newton_convergence_x7.ppm", "pictures/" + stem + "_convergence.ppm"])

# clean
print( "final cleaning..." )
subprocess.Popen(["make", "clean"], cwd=extraction_path, stdout=subprocess.DEVNULL).wait()


# feedback summary
if passed:
  print("submission passes script")
else:
  print("submission DOES NOT pass script")
