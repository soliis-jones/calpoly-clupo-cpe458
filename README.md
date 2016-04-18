# calpoly-clupo-cpe458

# Lab1
Team Members: Robert Jones & Travis Michael

Compiling:
  Running a simple "make" command will install the "TSP" executable to a bin directory in the present working directory.
  
Running:
  Our TSP executable takes in 2 positional command line arguments.
    1. The path and/or name of the input file.
    2. The number of iterations for each process (recommended 5x the number of nodes from input file)
    
  The input files should contain a line with the word "DIMENSION" followed by the total number of nodes in the current file on the same line. A line after that should include the text "NODE_COORD_SECTION" with all of the node coord information immediately following until EOF or the actual end of the file. Node coordinate lines should have 3 numbers per line which is in the format of [node_number, x_coordinate, y_coordinate]. Our program only accepts integer values for coordinate and node numbers.
  
  The output of our program will print the iteration number and process number when a new best tour value is received by the root node. Once the number of iterations has been reached the best path found will be printed along with the total distance. 

# End
