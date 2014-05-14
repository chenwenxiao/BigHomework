srcDirectory$ = "data\test"
dstDirectory$ = "pitch\test"

Create Strings as file list... list 'srcDirectory$'\*.wav
numberOfFiles = Get number of strings

for ifile to numberOfFiles

	select Strings list
	srcFileName$ = Get string... ifile
	Read from file... 'srcDirectory$'\'srcFileName$'
  
	dstFeatureFileName$ = srcFileName$ - ".wav" + ".pitch"
	filedelete 'dstDirectory$'\'dstFeatureFileName$'
	
	totalTime = Get total duration
#	clipTime = 0.876553288
	clipTime = 1.311927438
#	overlappingTime = 0.3541043084
	overlappingTime = 0.5282539683
	start = 0.0
	end = start + clipTime
	
	To Pitch... 0.01 75 600
	
	frameNum = 0
	
	repeat
	
		if end > totalTime
			end = totalTime
		endif
	
		mean = Get mean... start end Hertz
		median = Get quantile... start end 0.50(=median) Hertz
		std_deviation = Get standard deviation... start end Hertz
		min = Get minimum... start end Hertz Parabolic
		max = Get maximum... start end Hertz Parabolic
		range = max - min
		
		fileappend 'dstDirectory$'\'dstFeatureFileName$'
		... 'mean:4' 'median:4' 'std_deviation:4' 'min:4' 'max:4' 'range:4'
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
