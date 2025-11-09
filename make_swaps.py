import sys
import os
import subprocess
if len(sys.argv) != 5:
    print("Usage: python make_swaps.py <num> <size> <force_no_frag> <start_index>")
    sys.exit(1)

num = int(sys.argv[1])
size_gb = int(sys.argv[2])
force_no_frag = int(sys.argv[3])
start_index = int(sys.argv[4])
for i in range(num):
    fragmented = True
    exists = False
    while (fragmented and force_no_frag) or (not exists):
        subprocess.run(f"sudo fallocate -l {size_gb}G /swap_files/swapfile_{start_index+i}.swap", shell=True, check=True)
        # subprocess.run(f"sudo touch /swap_files/swapfile_{start_index+i}.swap", shell=True, check=True)
        # subprocess.run(f"sudo chattr +e /swap_files/swapfile_{start_index+i}.swap", shell=True, check=True)
        # subprocess.run(f"sudo fallocate -l {size_gb}G /swap_files/swapfile_{start_index+i}.swap", shell=True, check=True)
        # os.system(f"mkswap /swap_files/swapfile_{start_index+i}.swap")
        # os.system(f"swapon /swap_files/swapfile_{start_index+i}.swap")
        exists = True
        if force_no_frag == 1:
            proc = subprocess.run(f"filefrag /swap_files/swapfile_{start_index+i}.swap", stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
            output = proc.stdout.decode('utf-8')
            print(output)
            if "1 extent found" in output:
                print(f"swapfile_{start_index+i}.swap is contiguous")
                fragmented = False
            else:
                print(f"swapfile_{start_index+i}.swap is fragmented")
                fragmented = True
                os.system(f"sudo swapoff /swap_files/swapfile_{start_index+i}.swap")
                os.system(f"sudo rm /swap_files/swapfile_{start_index+i}.swap")

        