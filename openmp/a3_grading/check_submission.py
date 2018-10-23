#!/usr/bin/python3

import argparse, os, subprocess
from time import perf_counter

t = perf_counter()
subprocess.Popen("sleep 1", shell = True, stdout = subprocess.DEVNULL).wait(2)
tcorrection = perf_counter() - t - 1

def timeit(time_bd, repeat, cmd, path):
  for _ in range(repeat):
    t = perf_counter()
    try:
      subprocess.Popen(cmd, shell = True, cwd=path, stdout = subprocess.DEVNULL).wait(2*time_bd)
      dt = perf_counter() - t - tcorrection
      print(cmd, "runtime", dt)
      if dt < time_bd:
        return True
    except subprocess.TimeoutExpired:
      pass
  return False

def check_runtime(t, s, path, time_bd, repeat):
  if os.path.exists(path + "/cells"):
    os.remove(path + "/cells")
  os.symlink("/home/hpc2018/a3_grading/test_data/cell_e{}".format(s), path + "/cells")

  if not timeit(time_bd, repeat, "./cell_distance -t{} ".format(t), path):
    print( "TOO SLOW FOR -t{} cell_e{}".format(t,s) )
    return False

  return True
  
def check_runtimes(path):
  return (   check_runtime( 1, 4, path, 0.41, 10)
         and check_runtime( 5, 5, path, 8.2,  5)
         and check_runtime(10, 5, path, 4.1, 10)
         and check_runtime(20, 5, path, 2.6, 10)
         )


def check_correctness(distreffile, distfile):
  dist_ref = dict()
  dist = dict()
  for l in open(distreffile, 'r').readlines():
    l = l.split(" ")
    dist_ref[int(100*float(l[0].strip()))] = int(l[1])
  
  for l in open(distfile, 'r').readlines():
    l = l.split(" ")
    dist[int(100*float(l[0].strip()))] = int(l[1])

  invtolerance = 100
  abstolerance = 100

  keys = set(dist_ref.keys()).union(set(dist.keys()))
  for k in keys:
    val = dist.get(k,0) + dist.get(k+1,0) + dist.get(k-1,0)
    val_ref = dist_ref.get(k,0) + dist_ref.get(k+1,0) + dist_ref.get(k-1,0)
    if ( abs(val - val_ref) > abstolerance and
         abs(val - val_ref) * invtolerance > val_ref
       ) :
      return k

  return True


passed = True

subprocess.run(["mkdir", "-p", "extracted"])
subprocess.run(["mkdir", "-p", "reports"])
subprocess.run(["mkdir", "-p", "distances"])


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
subprocess.Popen(["make", "cell_distance"], cwd=extraction_path).wait()
subprocess.Popen(["make", "clean"], cwd=extraction_path).wait()
if not is_valid_folder(extraction_path): 
  print("ADDITIONAL FILES AFTER BUILD CLEAN")
  passed = False
subprocess.Popen(["make", "cell_distance"], cwd=extraction_path, stdout=subprocess.DEVNULL).wait()

# check times
print( "checking runtimes..." )
passed = passed and check_runtimes(extraction_path)


# create distances
print( "checking correctness..." )
if os.path.exists(extraction_path + "/cells"):
  os.remove(extraction_path + "/cells")
os.symlink("/home/hpc2018/a3_grading/test_data/cell_e5", extraction_path + "/cells")
with open("distances/" + stem, 'w') as f:
  subprocess.Popen("./cell_distance -t10", shell=True, cwd=extraction_path, stdout = f).wait(10)

correct = check_correctness("/home/hpc2018/a3_grading/test_data/dist_e5", "distances/" + stem)
correct_alt = check_correctness("/home/hpc2018/a3_grading/test_data/dist_e5_alt", "distances/" + stem)
if not (correct is True) and not (correct_alt is True):
  print( "WRONG VALUE AT {}/100 for dist_e5".format(correct) )
  print( "WRONG VALUE AT {}/100 for dist_e5_alt".format(correct_alt) )
  passed = False

# clean
print( "final cleaning..." )
subprocess.Popen(["make", "clean"], cwd=extraction_path, stdout=subprocess.DEVNULL).wait()


# feedback summary
if passed:
  print("submission passes script")
else:
  print("submission DOES NOT pass script")
