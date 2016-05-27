#! /bin/python
import subprocess

## benchmark variables
MHZ   = 500
HREP  = 100000
K1REP = 2

HOST="./search-x86"
SIM='./host_main output.mpk'
bmtime=0.0
bytecmp=0.0

def runbench(cmd, mhz, key_sz, val_sz, blk_sz, rep_cnt, dcache):
    try:
        out = subprocess.check_output((cmd + " %d %d %d %d %d %d") %
                                  (mhz, key_sz, val_sz, blk_sz, rep_cnt, dcache),
                                  shell = True)
        c = compile(out, "dummy", "exec")
        eval(c, globals())
    except SyntaxError:
        return (-1,-1)

    return (bmtime, bytecmp)

# the headings of the CSV file
print "K1 processor MHZ ", MHZ
print "blk size, key size, value size, bytes compared, host lat (no dcache), host latency, k1 latency, x factor, x factor (no dcache)"

for blk_sz in [16*1024, 64*1024, 128*1024, 512*1024, 1024*1024]:
    for key_sz in [8, 10, 100, 512, 1024, 4*1024, 8*1024, 16*1024]:
        for value_sz in [100, 500, 1024, 32*1024, 64*1024]:
            if blk_sz < (key_sz + value_sz + 16):
                continue
            (h_tm, h_by) = runbench(HOST, MHZ, key_sz, value_sz, blk_sz, HREP, 0)
            (k_tm, k_by) = runbench(SIM, MHZ, key_sz, value_sz, blk_sz, K1REP, 0)
            (h2_tm, h2_by) = runbench(HOST, MHZ, key_sz, value_sz, blk_sz, HREP, 1)
            if k_by != -1:
                assert(h_by == k_by)
                h_lat = h_tm/HREP
                k_lat = k_tm/K1REP
                h2_lat = h2_tm/HREP
                print "%d, %d, %d, %d, %f, %f, %f, %f, %f"%(blk_sz, key_sz, value_sz, h_by,
                                                            h2_lat,h_lat, k_lat, k_lat/h_lat, k_lat/h2_lat)
            else:
                print "Test failed on K1 (Out of memory): %d, %d, %d"%(blk_sz, key_sz, value_sz)
