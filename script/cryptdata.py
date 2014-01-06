import os
import platform
import zipfile

sysname = platform.system();
exesuffix = ".exe" if sysname=="Windows" else ""
cryptexe  = "../tools/encrypt/encrypt" + exesuffix

#zipfile.
     
def cryptfile(key, src, dst, dst_check):
    cryptcmd = cryptexe + " " + src + " " + dst + " " + dst_check + " '" + key + "'" 
    os.system(cryptcmd)

reskey = "abcdefghijklmnopqrstuvwxyz*&_ #=" #supported characters: [a-z] * & (space) # _ =
cryptfile(reskey, "../game_data.zip", "../res.dat", "../game_data_decrypted.zip");

if sysname == "Darwin":
    statsexe = "../tools/bytestats/stats"
    os.system(statsexe + " ../res.dat ../game_data.zip")
