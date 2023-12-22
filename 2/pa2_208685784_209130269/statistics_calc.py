import statistics


input_files_cnt = 17
output_file = open("packets_statistics.csv", 'w') # Create packet output file
output_file.write("Run Id,Average,Median,Maximal,StdEv\n") # Write table headers
for i in range(input_files_cnt): # Run through each input file
    filename = 'out_script_{0}.txt'.format(i+1) # Name of input file
    stay_times = []
    with open(filename, 'r') as file: # Open input file
        for line in file: # For each line in the file
            row = line.strip().split() # Split line by ' '
            arrival_time = int(row[0]) # Get arrival time
            depart_time = int(row[3]) # Get depart time
            stay_time = depart_time - arrival_time # Calculate stay time
            stay_times.append(stay_time) # Add stay time to list
    
    average_stay_time = sum(stay_times) / len(stay_times) # Calculate average
    median_stay_time = statistics.median(stay_times) # Calculate median
    max_stay_time = max(stay_times) # Calculate maximal
    std_deviation = statistics.stdev(stay_times) # Calculate stdev
    output_file.write("{0},{1},{2},{3},{4}\n".format(i+1, average_stay_time, median_stay_time, max_stay_time, std_deviation)) # Write data to output file
output_file.close()

output_file = open("buffers_statistics.csv", 'w') # Create buffer output file
output_file.write("Run Id,Average,Maximal,StdEv\n") # Write table headers
for i in range(input_files_cnt): # Run through each input file
    filename = '{0}.log'.format(i+1) # Name of input file
    buffers_sizes = []
    with open(filename, 'r') as file: # Open input file
        for line in file: # For each line in the file
            row = line.strip().split() # Split line by ' '
            size = int(row[3]) # Get buffer size
            buffers_sizes.append(size) # Add buffer size to list

    average_buffer_size = sum(buffers_sizes) / len(buffers_sizes) # Calculate average
    max_buffer_size = max(buffers_sizes) # Calculate maximal
    std_deviation = statistics.stdev(buffers_sizes) #Calculate stdev
    output_file.write("{0},{1},{2},{3},\n".format(i+1, average_buffer_size, max_buffer_size, std_deviation)) # Write data to output file
output_file.close()