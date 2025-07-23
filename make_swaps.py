import sys
import os
import subprocess
if len(sys.argv) != 4:
    print("Usage: python make_swaps.py <num> <size> <force_no_frag>")
    sys.exit(1)

num = int(sys.argv[1])
size = int(sys.argv[2])
force_no_frag = int(sys.argv[3])
for i in range(num):
    fragmented = True
    exists = False
    while (fragmented and force_no_frag) or (not exists):
        os.system(f"dd if=/dev/zero of=/scratch/vma_swaps/swapfile_{i+1+132}.swap bs=1G count={size} status=progress")
        os.system(f"mkswap /scratch/vma_swaps/swapfile_{i+1+132}.swap")
        # os.system(f"swapon /scratch/vma_swaps/swapfile_{i+1+132}.swap")
        exists = True
        if force_no_frag == 1:
            proc = subprocess.run(f"filefrag /scratch/vma_swaps/swapfile_{i+1+132}.swap", stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
            output = proc.stdout.decode('utf-8')
            print(output)
            if "1 extent found" in output:
                print(f"swapfile_{i+1+132}.swap is contiguous")
                fragmented = False
            else:
                print(f"swapfile_{i+1+132}.swap is fragmented")
                fragmented = True
                os.system(f"sudo swapoff /scratch/vma_swaps/swapfile_{i+1+132}.swap")
                os.system(f"rm /scratch/vma_swaps/swapfile_{i+1+132}.swap")

        