#Python script to create 3 files
#Files contains 10 random lowercase letters
#At the end of the script, the product of 2 random numbers is displayed

import random
import string

print
#length of the random word
wordLength = 10;

#number of files to create
numFiles = 3;

#function returns 10 random lowercase letters
def randomLetters(len):
		return ''.join(random.choice(string.ascii_lowercase) for i in range(len))

#Loop gets random letters, opens a file, writes to file
#then displays the contents of the file to the console
for i in range(numFiles):
	randomWords = randomLetters(wordLength)

	fileName = "random" + str(i) + ".txt"
	rFile = open(fileName, 'w')
	rFile.write(randomWords)
	rFile.close()

	print("file: " + fileName + " contains " + randomWords)

print
#produces 2 random numbers between 1 and 42
num1 = random.randint(1, 42)
num2 = random.randint(1, 42)

#prints random numbers
print "Random number 1 is: ", num1
print "Random number 2 is: ", num2

#Finds and prints product of the 2 random numbers
product = num1 * num2
print"The product of the 2 numbers is ", product
print
