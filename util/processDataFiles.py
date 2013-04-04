# Rob Hussey 4/3/2013
# Process each file in the data directory (txt extension)
# Split the data by DES, average the data for each DES
import glob
import matplotlib.pyplot as plt

NUMSEED = 6
RUNSPERSEED = 16
labels = ['DES-{0}'.format(i + 1) for i in range(RUNSPERSEED + 1)]

for fname in glob.glob("../data/*.txt"):
	print fname
	with open(fname, 'r') as dataFile:
		dataLine = dataFile.readlines()[-2] #second to last line has final data
		dataAllDES = dataLine[4:].strip().split('\t')
		dataAverages = []
		for i in range(0, NUMSEED * RUNSPERSEED, NUMSEED):
			dataDES = [float(item) for item in dataAllDES[i:i+NUMSEED]]
			avgDataDES = sum(dataDES) / len(dataDES)
			dataAverages.append(avgDataDES)
			plt.plot(avgDataDES, 'o')
		print dataAverages
		for label, y in zip(labels, dataAverages):
			plt.annotate(label, xy = (0, y), xytext = (-20, 20), textcoords = 'offset points', ha = 'right', va = 'bottom',
				bbox = dict(boxstyle = 'round,pad=0.5', fc = 'yellow', alpha = 0.5), arrowprops = dict(arrowstyle = '->', connectionstyle = 'arc3,rad=0'))
		plt.legend(str(range(1, RUNSPERSEED+1)).strip('[').strip(']').split(','))
		plt.show()