# Rob Hussey 4/17/2013
# Note: This script was written for a specific situation
#  for a particular project. It will likely not be useful
#  in other circumstances.
# Process each file in the data directory (txt extension)
# Split the data by DES, average the data for each DES
import glob, math, re

LARP_NUMSEED = 5
GEOP_NUMSEED = 5
LARP_RUNS = 36
GEOP_RUNS = 18
PROTS = 3 # static, rotate, aodv or distance, zone, aodv
TOTAL_RUNS = 0
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
		geop = False
		print fname[8:]
		dataLine = "" # a full line (RUNSPERSEED length) of data
		dataALLDES = [] # the line of data split into DES
		
		if (fname[-8:-4] == "LARP"):
			NUMSEED = LARP_NUMSEED
			RUNS = LARP_RUNS
			TOTAL_RUNS = NUMSEED * RUNS
			
		elif (fname[-8:-4] == "GEOP"):
			NUMSEED = GEOP_NUMSEED
			RUNS = GEOP_RUNS
			TOTAL_RUNS = NUMSEED * RUNS * PROTS
			geop = True

		# get data line and split into list
		# if the filter was sample_sum, just use the second to last line
		if (fname[-12:-9] == "SUM"):
			dataLine = dataFile.readlines()[-2][4:].strip().split('\t') #second to last line has final data for SUM files
			print len(dataLine)
			for val in dataLine:
				dataALLDES.append(val)
		else: # average filter
			dataLines = dataFile.readlines()
			for i in range(0, TOTAL_RUNS):
				dataALLDES.append("99999999") # some value large enough
			for i in range(0, len(dataLines)): # find the min avg values
				currLine = dataLines[i].strip().split('\t')[1:] # 1: since first column is time
				for j in range(0, TOTAL_RUNS):
					currValue = currLine[j]
					if (currValue < dataALLDES[j] and isDecimal(currValue)): 
						dataALLDES[j] = currValue
						
		avgAllDES = [] # all des averaged across seed\
		popSD = []
		resultsFile = open('..\\data\\' + fname[11:-3] + 'csv', 'a')
		
		# find the average across each seed and write the result to the results file
		for i in range(0, TOTAL_RUNS, NUMSEED):
			if (i > 0):
				resultsFile.write(',')
			dataDES = [float(item) for item in dataALLDES[i:i+NUMSEED]]
			dataDES.sort()
			dataDES = dataDES[1:-1] # remove max and min after sorting
			avgDataDES = sum(dataDES) / len(dataDES)
			diffALLDES = [math.pow((item - avgDataDES), 2) for item in dataDES] # for confidence interval calculation
			popSD.append(math.sqrt(sum(diffALLDES) / len(diffALLDES))) # population SD
			avgAllDES.append(avgDataDES)
			resultsFile.write(str(avgDataDES))
		
		resultsFile.write('\n')
		# reorder in tabular form such that columns = alpha and rows = beta
		if not geop:
			resultsFile.write(",1.0,1.025,1.05,1.075,1.1,1.125\n")
			for i in range(0, BETAS):
				resultsFile.write(str(0.05 * i)+','+str(avgAllDES[i])+','+str(avgAllDES[i+BETAS])+','+str(avgAllDES[i+BETAS*2])+','+
					str(avgAllDES[i+BETAS*3])+','+str(avgAllDES[i+BETAS*4])+','+str(avgAllDES[i+BETAS*5])+'\n')
			for i in range(0, BETAS):
				resultsFile.write(','+str(popSD[i])+','+str(popSD[i+BETAS])+','+str(popSD[i+BETAS*2])+','+
					str(popSD[i+BETAS*3])+','+str(popSD[i+BETAS*4])+','+str(popSD[i+BETAS*5])+'\n')
			resultsFile.write('\n')
		else:
			resultsFile.write(",10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180\n")
			for i in range(0, RUNS):
				resultsFile.write(',' + str(avgAllDES[i]))
			resultsFile.write("\n\n,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180\n")
			for i in range(RUNS, RUNS*2):
				resultsFile.write(',' + str(avgAllDES[i]))
			resultsFile.write("\n\n,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180\n")
			for i in range(RUNS*2, RUNS*3):
				resultsFile.write(',' + str(avgAllDES[i]))
		resultsFile.write('\n\n\n\n\n\n\n\n\n\n\n\n') # space for graphs

			