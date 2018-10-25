#!/usr/bin/python3

import argparse, os, subprocess
from time import perf_counter


def timeit(time_bd, repeat, cmd, path):
  for _ in range(repeat):
    t = perf_counter()
    subprocess.Popen(cmd, shell = True, cwd=path, stdout = subprocess.DEVNULL).wait(2*time_bd)
    if perf_counter() - t < time_bd:
      return True
  return False

def check_runtime(width, height, i, d, n, path, time_bd, repeat):
  if not timeit(time_bd, repeat,
                "./heat_diffusion {} {} -i{} -d{} -n{}".format(width, height, i, d, n), path):
    print( "TOO SLOW FOR {} {} -i{} -d{} -n{}".format(width, height, i, d, n) )
    return False

  return True

def check_runtimes(path):
  return (   check_runtime( 100    , 100   , "1e20" , "0.01" , 100000 , path , 1.7 , 10 )
         and check_runtime( 10000  , 10000 , "1e10" , "0.02" , 1000   , path , 98  , 3 )
         and check_runtime( 100000 , 100   , "1e10" , "0.6"  , 200    , path , 1.4 , 3 )
         )


passed = True

subprocess.run(["mkdir", "-p", "extracted"])
subprocess.run(["mkdir", "-p", "reports"])


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
     f.endswith(".incl") or f.endswith(".cl") or
     f.endswith(".cc") or f.endswith(".c") or
     f.endswith(".hh") or f.endswith(".h") )
is_valid_folder = lambda folder: (
  all( "material" in root or "reportextra" in root
        or all(map(is_valid_file, files))
        for (root, _, files) in os.walk(folder) ) )

print( "checking for additional files..." )
if not is_valid_folder(extraction_path): 
  print("ADDITIONAL FILES IN TAR")
  passed = False

# build clean build
print( "bulding and cleaning..." )
subprocess.Popen(["make", "heat_diffusion"], cwd=extraction_path).wait()
subprocess.Popen(["make", "clean"], cwd=extraction_path).wait()
if not is_valid_folder(extraction_path): 
  print("ADDITIONAL FILES AFTER BUILD CLEAN")
  passed = False
subprocess.Popen(["make", "heat_diffusion"], cwd=extraction_path, stdout=subprocess.DEVNULL).wait()


# check times
print( "checking runtimes..." )
passed = passed and check_runtimes(extraction_path)


# clean
print( "final cleaning..." )
subprocess.Popen(["make", "clean"], cwd=extraction_path, stdout=subprocess.DEVNULL).wait()


# feedback summary
if passed:
  print("submission passes script")
else:
  print("submission DOES NOT pass script")
