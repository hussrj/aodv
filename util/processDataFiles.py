# Rob Hussey 4/3/2013
# Process each file in the data directory (txt extension)
# Split the data by DES, average the data for each DES
import glob

RUNSPERSEED = 100

for fname in glob.glob("../data/*.txt"):
	print fname
	with open(fname, 'r') as dataFile:
		dataLine = dataFile.readlines()[-2] #second to last line has final data
		dataAllDES = dataLine[4:].strip().split('\t')
		for i in range(RUNSPERSEED):
			dataDES = [float(item) for item in dataAllDES[i::RUNSPERSEED]]
			print 'DES', (i+1), '=', sum(dataDES) / len(dataDES)