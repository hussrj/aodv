# Rob Hussey 4/17/2013
# Process each file in the data directory (txt extension)
# Split the data by DES, average the data for each DES
import glob
import re

LARP_NUMSEED = 7
GEOP_NUMSEED = 7
LARP_RUNSPERSEED = 36
GEOP_RUNSPERSEED = 36
GEOP_RUNSPERPROT = 18
BETAS = 6

# check if string is decimal - used when finding min(avg(dataDES))
def isDecimal(string):
	return re.match("^[0-9]*\.?[0-9]*$", string) != None

# explicitly clear all csv result files
for fname in glob.glob("../data/*.csv"):
	with open(fname, 'w') as dataFile:
		dataFile.write('')

# process each data file and create the results file
for fname in glob.glob("../data/*.txt"):
	with open(fname, 'r') as dataFile:
		print fname[11:]
		dataLine = "" # a full line (RUNSPERSEED length) of data
		dataALLDES = [] # the line of data split into DES
		
		if (fname[-8:-4] == "LARP"):
			NUMSEED = LARP_NUMSEED
			RUNSPERSEED = LARP_RUNSPERSEED

		# get data line and split into list
		# if the filter was sample_sum, just use the second to last line
		if (fname[-12:-9] == "SUM"):
			dataLine = dataFile.readlines()[-2][4:].strip().split('\t') #second to last line has final data for SUM files
			for val in dataLine:
				dataALLDES.append(val)
		else: # average filter
			dataLines = dataFile.readlines()
			for i in range(0, NUMSEED * RUNSPERSEED):
				dataALLDES.append("99999999") # some value large enough
			for i in range(0, len(dataLines)): # find the min avg values
				currLine = dataLines[i].strip().split('\t')[1:] # 1: since first column is time
				for j in range(0, NUMSEED * RUNSPERSEED):
					currValue = currLine[j]
					if (currValue < dataALLDES[j] and isDecimal(currValue)): 
						dataALLDES[j] = currValue
						
		avgAllDES = [] # all des averaged across seed
		resultsFile = open('..\\data\\' + fname[11:-3] + 'csv', 'a')
		
		# find the average across each seed and write the result to the results file
		for i in range(0, NUMSEED * RUNSPERSEED, NUMSEED):
			if (i > 0):
				resultsFile.write(',')
			
			dataDES = [float(item) for item in dataALLDES[i:i+NUMSEED-1]]
			avgDataDES = sum(dataDES) / len(dataDES)
			avgAllDES.append(avgDataDES)
			resultsFile.write(str(avgDataDES))
		
		resultsFile.write('\n')
		# reorder in tabular form such that columns = alpha and rows = beta
		resultsFile.write(",1.0,1.025,1.05,1.075,1.1,1.125\n")
		for i in range(0, BETAS):
			resultsFile.write(str(0.05 * i)+','+str(avgAllDES[i])+','+str(avgAllDES[i+BETAS])+','+str(avgAllDES[i+BETAS*2])+','+
						str(avgAllDES[i+BETAS*3])+','+str(avgAllDES[i+BETAS*4])+','+str(avgAllDES[i+BETAS*5])+'\n')
		resultsFile.write('\n\n\n\n\n\n\n\n\n\n\n\n') # space for graphs
			