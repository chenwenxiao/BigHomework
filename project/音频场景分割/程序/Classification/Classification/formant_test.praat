srcDirectory$ = "data\test"
dstDirectory$ = "formant\test"

Create Strings as file list... list 'srcDirectory$'\*.wav
numberOfFiles = Get number of strings

for ifile to numberOfFiles

	select Strings list
	srcFileName$ = Get string... ifile
	Read from file... 'srcDirectory$'\'srcFileName$'
  
	dstFeatureFileName$ = srcFileName$ - ".wav" + ".formant"
	filedelete 'dstDirectory$'\'dstFeatureFileName$'
	
	totalTime = Get total duration
#	clipTime = 0.876553288
	clipTime = 1.311927438
#	overlappingTime = 0.3541043084
	overlappingTime = 0.5282539683
	start = 0.0
	end = start + clipTime
	
	To Formant (burg)... 0.01 5 8000 0.025 50
	
	frameNum = 0
	
	repeat
	
		if end > totalTime
			end = totalTime
		endif
	
		mean_1 = Get mean... 1 start end Hertz
		mean_2 = Get mean... 2 start end Hertz
		mean_3 = Get mean... 3 start end Hertz
		mean_4 = Get mean... 4 start end Hertz
		
		median_1 = Get quantile... 1 start end Hertz 0.50
		median_2 = Get quantile... 2 start end Hertz 0.50
		median_3 = Get quantile... 3 start end Hertz 0.50
		median_4 = Get quantile... 4 start end Hertz 0.50
		
		std_deviation_1 = Get standard deviation... 1 start end Hertz
		std_deviation_2 = Get standard deviation... 2 start end Hertz
		std_deviation_3 = Get standard deviation... 3 start end Hertz
		std_deviation_4 = Get standard deviation... 4 start end Hertz
		
		min_1 = Get minimum... 1 start end Hertz None
		min_2 = Get minimum... 2 start end Hertz None
		min_3 = Get minimum... 3 start end Hertz None
		min_4 = Get minimum... 4 start end Hertz None
		
		max_1 = Get maximum... 1 start end Hertz None
		max_2 = Get maximum... 2 start end Hertz None
		max_3 = Get maximum... 3 start end Hertz None
		max_4 = Get maximum... 4 start end Hertz None
		
		range_1 = max_1 - min_1
		range_2 = max_2 - min_2
		range_3 = max_3 - min_3
		range_4 = max_4 - min_4
		
		fileappend 'dstDirectory$'\'dstFeatureFileName$'
		... 'mean_1:4' 'median_1:4' 'std_deviation_1:4' 'min_1:4' 'max_1:4' 'range_1:4'
		... 'mean_2:4' 'median_2:4' 'std_deviation_2:4' 'min_2:4' 'max_2:4' 'range_2:4'
		... 'mean_3:4' 'median_3:4' 'std_deviation_3:4' 'min_3:4' 'max_3:4' 'range_3:4'
		... 'mean_4:4' 'median_4:4' 'std_deviation_4:4' 'min_4:4' 'max_4:4' 'range_4:4'
		...'newline$'
	
		if end <> totalTime
			start = end - overlappingTime
			end = start + clipTime
			frameNum = frameNum + 1
		endif
	
	until end = totalTime

endfor

select Strings list
Remove
