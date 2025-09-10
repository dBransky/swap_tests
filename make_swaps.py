import sys
import os
import subprocess
if len(sys.argv) != 4:
    print("Usage: python make_swaps.py <num> <size> <force_no_frag>")
    sys.exit(1)

num = int(sys.argv[1])
size_gb = int(sys.argv[2])
force_no_frag = int(sys.argv[3])
for i in range(num):
    fragmented = True
    exists = False
    while (fragmented and force_no_frag) or (not exists):
        subprocess.run(f"fallocate -l {size_gb}G /swap_files/swapfile_{i+1}.swap", shell=True, check=True)
        # os.system(f"mkswap /swap_files/swapfile_{i+1}.swap")
        # os.system(f"swapon /swap_files/swapfile_{i+1}.swap")
        exists = True
        if force_no_frag == 1:
            proc = subprocess.run(f"filefrag /swap_files/swapfile_{i+1}.swap", stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
            output = proc.stdout.decode('utf-8')
            print(output)
            if "1 extent found" in output:
                print(f"swapfile_{i+1}.swap is contiguous")
                fragmented = False
            else:
                print(f"swapfile_{i+1}.swap is fragmented")
                fragmented = True
                os.system(f"sudo swapoff /swap_files/swapfile_{i+1}.swap")
                os.system(f"rm /swap_files/swapfile_{i+1}.swap")

        