import os

key = "abcdefghijklmnopqrstuvwxyz*&_ #=" #supported characters: [a-z] * & (space) # _ =
cryptexe  = "../tools/encrypt/encrypt"
indata    = "../game_data.zip"
checkdata = "../game_data_decrypted.zip"
outdata   = "../game.dat"

statexe   = "../tools/bytestats/stats"

cryptcmd = cryptexe + " " + indata + " " + outdata + " " + checkdata + " '" + key + "'" 
os.system(cryptcmd)
os.system(statexe + " " + outdata)
os.system(statexe + " " + checkdata)
