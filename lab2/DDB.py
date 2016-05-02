#Authors: Bobby Jones & Travis Michael

#imports go here
import sys
import csv
import leveldb

#functions



# We need to create database entries that have "Date" as the key, and a tuple of <volume, adjusted closing price> as the value.
# Example header: Date,Open,High,Low,Close,Volume,Adj Close


def main():
   f = open(sys.argv[1], 'rb')

   try:
      rownum = 0
      reader = csv.DictReader(f)  # creates the reader object
      for row in reader:      # iterates the rows of the file in order
         if rownum == 0:
            header = row
            print reader.fieldnames
         else
            print row            # prints each row
            dict[row['Date']] = {row['Volume'], row['Adj Close']}
   except csv.Error as e:
      sys.exit('file %s, line %d: %s' % (filename, reader.line_num, e))
   finally:
      f.close()               # closing

if __name__ == "__main__":
    main()